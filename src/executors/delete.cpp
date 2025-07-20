#include "global.h"
#include <iostream>

bool syntacticParseDELETE() {
    logger.log("syntacticParseDELETE");
    if (tokenizedQuery.size() != 7 || tokenizedQuery[1] != "FROM" ||
        tokenizedQuery[3] != "WHERE") {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = DELETE;
    parsedQuery.selectionRelationName = tokenizedQuery[2];
    parsedQuery.selectionFirstColumnName = tokenizedQuery[4];

    string binaryOperator = tokenizedQuery[5];
    if (binaryOperator == "<")
        parsedQuery.selectionBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.selectionBinaryOperator = GREATER_THAN;
    else if (binaryOperator == "<=")
        parsedQuery.selectionBinaryOperator = LEQ;
    else if (binaryOperator == ">=")
        parsedQuery.selectionBinaryOperator = GEQ;
    else if (binaryOperator == "==")
        parsedQuery.selectionBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.selectionBinaryOperator = NOT_EQUAL;
    else {
        cout << "SYNTAX ERROR: Invalid operator" << endl;
        return false;
    }

    parsedQuery.selectionIntLiteral = stoi(tokenizedQuery[6]);
    return true;
}

bool semanticParseDELETE() {
    logger.log("semanticParseDELETE");

    if (!tableCatalogue.isTable(parsedQuery.selectionRelationName)) {
        cout << "SEMANTIC ERROR: Table doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.selectionFirstColumnName,
                                          parsedQuery.selectionRelationName)) {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    Table *table = tableCatalogue.getTable(parsedQuery.selectionRelationName);
    // if (!table->indexed ||
    //     table->indexedColumn != parsedQuery.selectionFirstColumnName) {
    //     cout << "SEMANTIC ERROR: Table not indexed on the selected column"
    //          << endl;
    //     return false;
    // }

    return true;
}

static const int BUCKET_COUNT = BLOCK_COUNT_P2 - 1;

static int findFirstPageGE(Table *I, int value) {
    int lo = 0, hi = I->blockCount - 1, ans = I->blockCount;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        Page P = bufferManager.getPage(I->tableName, mid);
        P.resize(I->rowsPerBlockCount[mid]);
        int lastKey = P.getRow(P.getRowCount() - 1)[0];
        if (lastKey < value) {
            lo = mid + 1;
        } else {
            ans = mid;
            hi = mid - 1;
        }
    }
    return ans;
}

static int findLastPageLE(Table *I, int value) {
    int lo = 0, hi = I->blockCount - 1, ans = -1;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        Page P = bufferManager.getPage(I->tableName, mid);
        P.resize(I->rowsPerBlockCount[mid]);
        int firstKey = P.getRow(0)[0];
        if (firstKey > value) {
            hi = mid - 1;
        } else {
            ans = mid;
            lo = mid + 1;
        }
    }
    return ans;
}

// Function to determine if a row should be removed based on the predicate
bool shouldRemoveRow(int key, int value, BinaryOperator op) {
    switch (op) {
    case EQUAL:
        return (key == value);
    case NOT_EQUAL:
        return (key != value);
    case LESS_THAN:
        return (key < value);
    case LEQ:
        return (key <= value);
    case GREATER_THAN:
        return (key > value);
    case GEQ:
        return (key >= value);
    default:
        return false;
    }
}

void executeDELETE() {
    logger.log("executeDELETE");
    Table *T = tableCatalogue.getTable(parsedQuery.selectionRelationName);

    string idxName = "idx_" + T->tableName + "_" + parsedQuery.selectionFirstColumnName; //CHANGE
    string col = parsedQuery.selectionFirstColumnName;
    if(!T->hasIndexOn(col) || T->isIndexDirty(col))
    {
        bool ok = T->buildSecondaryIndex(col);
        if (!ok) {
            cout << "Index build failed" << endl;
            cout << "No index: DELETE operation not supported without index\n";
            return;
        }
    }

    // open the index table
    Table *I = tableCatalogue.getTable(idxName);
    auto op = parsedQuery.selectionBinaryOperator;
    int v = parsedQuery.selectionIntLiteral;

    unordered_set<int> alreadyDeletedPages;

    int startPg = 0, endPg = I->blockCount - 1;
    // for equality we can narrow both
    if (op == EQUAL) {
        startPg = findFirstPageGE(I, v);
        endPg = findLastPageLE(I, v);
    }
    // for >= or >
    else if (op == GEQ || op == GREATER_THAN) {
        int lb = findFirstPageGE(I, v + (op == GREATER_THAN ? 1 : 0));
        startPg = lb;
    }
    // for <= or <
    else if (op == LEQ || op == LESS_THAN) {
        int rb = findLastPageLE(I, v - (op == LESS_THAN ? 1 : 0));
        endPg = rb;
    }

    // scan the relevant index pages and delete rows
    for (int pg = startPg; pg <= endPg; ++pg) {
        if (pg < 0 || pg >= I->blockCount)
            continue;

        Page P = bufferManager.getPage(I->tableName, pg);

        int rowsInPage = P.getRowCount();
        vector<vector<int>> updatedRows; // Store rows that are not deleted

        for (int r = 0; r < rowsInPage; ++r) {
            auto idxRow = P.getRow(r);
            int key = idxRow[0];

            // apply the predicate
            bool remove = shouldRemoveRow(key, v, op);
            if (!remove) {
                updatedRows.push_back(idxRow);
                continue;
            }

            if (alreadyDeletedPages.contains(idxRow[1])) {
                continue;
            }

            // fetch the data page and delete all the rows matching the key
            int deletedRows = 0;
            Page dataPage = bufferManager.getPage(T->tableName, idxRow[1]);
            vector<vector<int>> dataRows;
            for (int dr = 0; dr < dataPage.getRowCount(); ++dr) {
                auto dataRow = dataPage.getRow(dr);
                int columnIndex = T->getColumnIndex(T->indexedColumn);
                bool remove = shouldRemoveRow(dataRow[columnIndex], v, op);
                if (!remove) {
                    dataRows.push_back(dataRow);
                } else {
                    deletedRows++;
                }
            }

            // update the data page with remaining rows
            bufferManager.removePage(T->tableName, idxRow[1]);
            T->rowsPerBlockCount[idxRow[1]] -= deletedRows;
            bufferManager.writePage(T->tableName, idxRow[1], dataRows,
                                    dataRows.size());
            alreadyDeletedPages.insert(idxRow[1]);
            T->rowCount -= deletedRows;
        }
    }
    // recreate index logic!
    T->markIndexDirty(parsedQuery.selectionFirstColumnName);
}

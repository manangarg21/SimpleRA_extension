#include "global.h"
#include <iostream>

// UPDATE MARKS WHERE stud_id >= 10 SET chemistry_marks = 1
bool syntacticParseUPDATE() {
    logger.log("syntacticParseUPDATE");
    if (tokenizedQuery.size() != 10 || tokenizedQuery[2] != "WHERE" ||
        tokenizedQuery[6] != "SET" || tokenizedQuery[8] != "=") {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = UPDATE;
    parsedQuery.selectionRelationName = tokenizedQuery[1];
    parsedQuery.selectionFirstColumnName = tokenizedQuery[3];
    parsedQuery.updateColumnName = tokenizedQuery[7];
    parsedQuery.updateValue = stoi(tokenizedQuery[9]);

    string binaryOperator = tokenizedQuery[4];
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

    parsedQuery.selectionIntLiteral = stoi(tokenizedQuery[5]);
    return true;
}

bool semanticParseUPDATE() {
    logger.log("semanticParseUPDATE");

    if (!tableCatalogue.isTable(parsedQuery.selectionRelationName)) {
        cout << "SEMANTIC ERROR: Table doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.selectionFirstColumnName,
                                          parsedQuery.selectionRelationName)) {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.updateColumnName,
                                          parsedQuery.selectionRelationName)) {
        cout << "SEMANTIC ERROR: Column to update doesn't exist in relation"
             << endl;
        return false;
    }

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

void executeUPDATE() {
    logger.log("executeUPDATE");
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

    // scan the relevant index pages and update rows
    for (int pg = startPg; pg <= endPg; ++pg) {
        if (pg < 0 || pg >= I->blockCount)
            continue;

        // Fetch Index Page.
        Page P = bufferManager.getPage(I->tableName, pg); // Use reference
        int rowsInPage = P.getRowCount();
        int conditionColumnIndex =
            T->getColumnIndex(parsedQuery.selectionFirstColumnName);
        int updateColumnIndex =
            T->getColumnIndex(parsedQuery.updateColumnName);

        for (int r = 0; r < rowsInPage; ++r) {
            auto row = P.getRow(r);
            int key = row[0];

            // apply the predicate
            bool update = false;
            switch (op) {
            case EQUAL:
                update = (key == v);
                break;
            case NOT_EQUAL:
                update = (key != v);
                break;
            case LESS_THAN:
                update = (key < v);
                break;
            case LEQ:
                update = (key <= v);
                break;
            case GREATER_THAN:
                update = (key > v);
                break;
            case GEQ:
                update = (key >= v);
                break;
            default:
                break;
            }
            if (!update)
                continue;
            
            Page datapage = bufferManager.getPage(T->tableName, row[1]);
            datapage.updateRow(row[2], updateColumnIndex, parsedQuery.updateValue);
            datapage.writePage(); // Write the updated page back to disk
            bufferManager.removePage(T->tableName, row[1]);
        }
    }
    // recreate index logic!
    T->markIndexDirty(parsedQuery.selectionFirstColumnName);
}

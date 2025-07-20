#include "global.h"
#include <iostream>
#include <sys/stat.h>

bool syntacticParseSEARCH() {
    logger.log("syntacticParseSEARCH");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[3] != "FROM" ||
        tokenizedQuery[5] != "WHERE") {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = SEARCH;
    parsedQuery.selectionResultRelationName = tokenizedQuery[0];
    parsedQuery.selectionRelationName = tokenizedQuery[4];
    parsedQuery.selectionFirstColumnName = tokenizedQuery[6];

    string binaryOperator = tokenizedQuery[7];
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

    parsedQuery.selectionIntLiteral = stoi(tokenizedQuery[8]);
    return true;
}

bool semanticParseSEARCH() {
    logger.log("semanticParseSEARCH");

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
        int lastKey = P.getRow(P.getRowCount()-1)[0];
        if (lastKey < value) {
            lo = mid + 1;
        } else {
            ans = mid;
            hi  = mid - 1;
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
            lo  = mid + 1;
        }
    }
    return ans;
}

// void executeSEARCH() {
//     logger.log("executeSEARCH");
//     Table *T = tableCatalogue.getTable(parsedQuery.selectionRelationName);

//     // no usable index? fallback
//     if (!(T->indexed &&
//           T->indexedColumn == parsedQuery.selectionFirstColumnName)) {
//         cout << "No index: falling back to SELECT scan\n";
//         executeSELECTION();
//         return;
//     }

//     BinaryOperator op = parsedQuery.selectionBinaryOperator;
//     int lit = parsedQuery.selectionIntLiteral;
//     auto &idx = T->secondaryIndex;

//     // Prepare the result relation
//     Table *R = new Table(parsedQuery.selectionResultRelationName, T->columns);

//     // Exact match
//     if (op == EQUAL) {
//         auto it = idx.find(lit);
//         if (it != idx.end()) {
//             for (auto &loc : it->second) {
//                 Page P = bufferManager.getPage(T->tableName, loc.first);
//                 R->writeRow(P.getRow(loc.second));
//             }
//         }
//     }
//     // Not equal
//     else if (op == NOT_EQUAL) {
//         for (auto &kv : idx) {
//             if (kv.first == lit)
//                 continue;
//             for (auto &loc : kv.second) {
//                 Page P = bufferManager.getPage(T->tableName, loc.first);
//                 R->writeRow(P.getRow(loc.second));
//             }
//         }
//     }
//     // Range queries
//     else {
//         // Determine start/end iterators
//         auto lo = (op == GREATER_THAN || op == GEQ)
//                       ? idx.lower_bound(lit + (op == GREATER_THAN ? 1 : 0))
//                       : idx.begin();
//         auto hi = (op == LESS_THAN || op == LEQ)
//                       ? idx.upper_bound(lit - (op == LESS_THAN ? 0 : 1))
//                       : idx.end();

//         for (auto it = lo; it != hi; ++it) {
//             for (auto &loc : it->second) {
//                 Page P = bufferManager.getPage(T->tableName, loc.first);
//                 R->writeRow(P.getRow(loc.second));
//             }
//         }
//     }

//     // finalize
//     R->blockify();
//     tableCatalogue.insertTable(R);
// }

void executeSEARCH() {
    logger.log("executeSEARCH");
    Table *T = tableCatalogue.getTable(parsedQuery.selectionRelationName);
    const string col = parsedQuery.selectionFirstColumnName;
    auto op = parsedQuery.selectionBinaryOperator;
    int literal      = parsedQuery.selectionIntLiteral;

    // 1) If no index on col or it's dirty → build/rebuild now
    if (!T->hasIndexOn(col) || T->isIndexDirty(col)) {
        // cout << "[auto-index] building secondary index on "
            //  << T->tableName << "(" << col << ")\n";
        bool ok = T->buildSecondaryIndex(col);
        if (!ok) {
            // if index build failed, fall back
            // cout << "Index build failed, falling back to full SELECT\n";
            executeSELECTION();
            return;
        }
        // mark clean
        T->markIndexRebuilt(const_cast<string&>(col));
    }

    // 2) Open the index table
    string idxName = "idx_" + T->tableName + "_" + col;
    Table *I = tableCatalogue.getTable(idxName);
    if (!I) {
        cout << "ERROR: index table not found, falling back to SELECT\n";
        executeSELECTION();
        return;
    }

    // 3) Prepare result table
    Table *R = new Table(parsedQuery.selectionResultRelationName, T->columns);

    // 4) Determine which index pages to scan
    int startPg = 0, endPg = I->blockCount - 1;
    if (op == EQUAL) {
        startPg = findFirstPageGE(I, literal);
        endPg   = findLastPageLE(I, literal);
    }
    else if (op == GREATER_THAN || op == GEQ) {
        int v = literal + (op==GREATER_THAN?1:0);
        startPg = findFirstPageGE(I, v);
    }
    else if (op == LESS_THAN || op == LEQ) {
        int v = literal - (op==LESS_THAN?1:0);
        endPg = findLastPageLE(I, v);
    }
    // NOT_EQUAL will scan full [0..end], but we’ll filter below

    // 5) Scan only the needed index pages
    for (int pg = startPg; pg <= endPg; ++pg) {
        if (pg < 0 || pg >= I->blockCount) continue;
        Page P = bufferManager.getPage(I->tableName, pg);
        int rowsInPg = I->rowsPerBlockCount[pg];
        for (int r = 0; r < rowsInPg; ++r) {
            auto idxRow = P.getRow(r);
            int key = idxRow[0];
            bool keep = false;
            switch (op) {
                case EQUAL:        keep = (key == literal); break;
                case NOT_EQUAL:    keep = (key != literal); break;
                case LESS_THAN:    keep = (key <  literal); break;
                case LEQ:          keep = (key <= literal); break;
                case GREATER_THAN: keep = (key >  literal); break;
                case GEQ:          keep = (key >= literal); break;
                default: break;
            }
            if (!keep) continue;

            // Each idxRow = [ key, dataPage, dataOffset ]
            int dataPg  = idxRow[1];
            int dataOff = idxRow[2];
            Page D = bufferManager.getPage(T->tableName, dataPg);
            R->writeRow(D.getRow(dataOff));
        }
    }

    // 6) Finalize and register result
    R->blockify();
    tableCatalogue.insertTable(R);
}
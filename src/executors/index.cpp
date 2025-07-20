#include "global.h"
#include <iostream>
/**
 * @brief
 * SYNTAX: INDEX ON column_name FROM relation_name USING indexing_strategy
 * indexing_strategy: ASC | DESC | NOTHING
 */
bool syntacticParseINDEX() {
    logger.log("syntacticParseINDEX");
    if (tokenizedQuery.size() != 7 || tokenizedQuery[1] != "ON" ||
        tokenizedQuery[3] != "FROM" || tokenizedQuery[5] != "USING") {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = INDEX;
    parsedQuery.indexColumnName = tokenizedQuery[2];
    parsedQuery.indexRelationName = tokenizedQuery[4];
    string indexingStrategy = tokenizedQuery[6];
    if (indexingStrategy == "BTREE")
        parsedQuery.indexingStrategy = BTREE;
    else if (indexingStrategy == "HASH")
        parsedQuery.indexingStrategy = HASH;
    else if (indexingStrategy == "NOTHING")
        parsedQuery.indexingStrategy = NOTHING;
    else {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseINDEX() {
    logger.log("semanticParseINDEX");
    if (!tableCatalogue.isTable(parsedQuery.indexRelationName)) {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    if (!tableCatalogue.isColumnFromTable(parsedQuery.indexColumnName,
                                          parsedQuery.indexRelationName)) {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    Table *table = tableCatalogue.getTable(parsedQuery.indexRelationName);
    if (table->indexed) {
        cout << "SEMANTIC ERROR: Table already indexed" << endl;
        return false;
    }
    return true;
}

static const int BUCKET_COUNT = BLOCK_COUNT_P2 - 1;

// void executeINDEX() {
//     logger.log("executeINDEX");
//     Table *T = tableCatalogue.getTable(parsedQuery.indexRelationName);
//     const string &col = parsedQuery.indexColumnName;
//     int colIdx = T->getColumnIndex(col);

//     if (T->indexed) {
//         cout << "ERROR: table already indexed\n";
//         return;
//     }

//     // Scan every page/row and insert into the ordered map
//     for (uint pg = 0; pg < T->blockCount; ++pg) {
//         Page P = bufferManager.getPage(T->tableName, pg);
//         int rowsInPage = T->rowsPerBlockCount[pg];
//         for (int r = 0; r < rowsInPage; ++r) {
//             auto row = P.getRow(r);
//             int key = row[colIdx];
//             T->secondaryIndex[key].emplace_back(pg, r);
//         }
//     }

//     // Mark metadata so SEARCH will use it
//     T->indexed = true;
//     T->indexedColumn = col;
//     T->indexingStrategy =
//         HASH; // reuse HASH enum to indicate “we built an index”

//     cout << "Secondary index built on " << T->tableName << "(" << col
//          << ") with " << T->secondaryIndex.size() << " distinct keys\n";
// }

void createIndex(Table *T, const string &col, bool IndexCommand=false){
    const string base = T->tableName;

    if (IndexCommand && !T->indexed) {
        // **DEBUG #1**: does the base table actually have pages?
        std::cout << "[debug] Base table `" << base
                  << "` has " << T->blockCount
                  << " pages and " << T->rowCount
                  << " rows\n";
    }

    if (T->indexed) {
        if (IndexCommand) {
            cout << "ERROR: table already indexed\n";
            return;
        } else {
            
            cout << "WARNING: table already indexed. Index changing\n";
        }
    }

    // 1) Create the new index table (temp CSV + header)
    const string idxName = "idx_" + base + "_" + col;
    Table *I = new Table(idxName, vector<string>{ col, "page", "offset" });
    tableCatalogue.insertTable(I);

    // 2) Stream every row of T into I
    int colIdx = T->getColumnIndex(col);
    for (uint pg = 0; pg < T->blockCount; ++pg) {
        Page P = bufferManager.getPage(base, pg);
        int rowsInPage = T->rowsPerBlockCount[pg];
        for (int r = 0; r < rowsInPage; ++r) {
            auto row = P.getRow(r);
            int key = row[colIdx];
            // **CORRECTED**: force a vector<int> so writeRow actually appends
            I->writeRow(vector<int>{ key, (int)pg, r });
        }
    }

    // **DEBUG #2**: count how many data lines got written
    {
      std::ifstream fin(I->sourceFileName);
      int lines = 0;  string dummy;
      while (std::getline(fin, dummy)) ++lines;
      // one line is header, so data rows = lines-1
      std::cout << "[debug] Wrote " << (lines>0?lines-1:0)
                << " entries into " << I->sourceFileName << "\n";
    }

    // 3) Materialize into pages
    if (!I->loadIndex()) {
      cout << "ERROR: failed to blockify index table\n";
      return;
    }

    // 4) External sort on the key column
    I->sort({ col }, { ASC });

    // 5) Mark the base table as indexed
    T->indexed            = true;
    T->indexedColumn      = col;
    T->indexingStrategy   = HASH;              // just to flag “we have an index”
    T->secondaryIndexName = idxName;

    // 6) Report how many pages you ended up with
    if(IndexCommand) {
        cout << "Built on-disk secondary index “" << idxName
        << "” sorted on " << col
        << " (" << I->blockCount << " pages)\n";
    }

}

void executeINDEX() {
    logger.log("executeINDEX");
    Table *T = tableCatalogue.getTable(parsedQuery.indexRelationName);
    const string col  = parsedQuery.indexColumnName;

    if (!T) {
        cout << "ERROR: table not found\n";
        return;
    }

    createIndex(T, col, true);
}
#include "global.h"
/**
 * @brief
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1, column_name2
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 8 || tokenizedQuery[5] != "ON")
    {
        cout << "SYNTAX ERROR"<< endl;
        // cout << tokenizedQuery.size() << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    // cout << "Join Result Relation Name: " << parsedQuery.joinResultRelationName << endl;
    parsedQuery.joinFirstRelationName = tokenizedQuery[3];
    // cout << "Join First Relation Name: " << parsedQuery.joinFirstRelationName << endl;
    parsedQuery.joinSecondRelationName = tokenizedQuery[4];
    // cout << "Join Second Relation Name: " << parsedQuery.joinSecondRelationName << endl;
    parsedQuery.joinFirstColumnName = tokenizedQuery[6];
    // cout << "Join First Column Name: " << parsedQuery.joinFirstColumnName << endl;
    parsedQuery.joinSecondColumnName = tokenizedQuery[7];
    // cout << "Join Second Column Name: " << parsedQuery.joinSecondColumnName << endl;
    // string binaryOperator = tokenizedQuery[7];
    // if (binaryOperator == "<")
    //     parsedQuery.joinBinaryOperator = LESS_THAN;
    // else if (binaryOperator == ">")
    //     parsedQuery.joinBinaryOperator = GREATER_THAN;
    // else if (binaryOperator == ">=" || binaryOperator == "=>")
    //     parsedQuery.joinBinaryOperator = GEQ;
    // else if (binaryOperator == "<=" || binaryOperator == "=<")
    //     parsedQuery.joinBinaryOperator = LEQ;
    // else if (binaryOperator == "==")
    parsedQuery.joinBinaryOperator = EQUAL;
    // else
    // {
    //     cout << "SYNTAX ERROR" << endl;
    //     return false;
    // }
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    Table *table1 = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *table2 = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    if (!table1->isColumn(parsedQuery.joinFirstColumnName) || !table2->isColumn(parsedQuery.joinSecondColumnName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}


const int PARTITION_COUNT = BLOCK_COUNT_P2 - 1;

void print_row(vector<int> row){
    for(int i=0; i<row.size(); i++){
        cout << row[i] << " ";
    }
    cout << endl;
}

void deleteTempHashFiles(string TableName, int partitionCount, vector<array<int, 2>>& temp_counter) {
    logger.log("deleteTempHashFiles");

    for (int bucket = 0; bucket < partitionCount; bucket++) {
        for (int subfile = 0; subfile < temp_counter[bucket][1]; subfile++) {
            string fileName = TableName + "_" + to_string(bucket);
            bufferManager.deleteHashFile(fileName, subfile);
        }
    }
}

void PartitionToHash(Table* tx, int joinix, int maxrowsx, vector<array<int, 2>> &tempx_counter, vector<vector<vector<int>>> &store, int which_rel){   
    Cursor cursor = tx->getCursor();
    vector<int> row = cursor.getNext();
    string which_rel_name = which_rel == 1 ? parsedQuery.joinFirstRelationName : parsedQuery.joinSecondRelationName;
    while(row.size()!=0){
        // print_row(row);
        int key = row[joinix];
        int bucket = key % PARTITION_COUNT; //TBD- Change if index needs changing
        if (bucket < 0) bucket = -bucket;   

        //Write row to appropriate bucket
        store[bucket].push_back(row);
        tempx_counter[bucket][0]++;
        if(tempx_counter[bucket][0] == maxrowsx){
            //Logic for writing to disk- to be implemented
            bufferManager.writeHashPage(which_rel_name+ "_" + to_string(bucket), tempx_counter[bucket][1], store[bucket], store[bucket].size());
            tempx_counter[bucket][0] = 0;
            tempx_counter[bucket][1]++;
            store[bucket].clear();
            // cout << "Debug: Partition " << bucket << " of relation " << parsedQuery.joinFirstRelationName << " written to disk, subfile " << temp1_counter[bucket][1] - 1 << endl;
        }
        row = cursor.getNext();
    }

    // Write any remaining rows in the store to disk for t2
    for (int bucket = 0; bucket < PARTITION_COUNT; bucket++) {
        if (tempx_counter[bucket][0]!=0) {
            // cout << "Debug: Bucket " << bucket << " of relation " << parsedQuery.joinFirstRelationName << " has " << temp1_counter[bucket][0] << " rows remaining before writing to disk." << endl;
            bufferManager.writeHashPage(which_rel_name+"_"+to_string(bucket), tempx_counter[bucket][1], store[bucket], store[bucket].size());
            tempx_counter[bucket][0] = 0;
            tempx_counter[bucket][1]++;
            store[bucket].clear();
        }
    }
}

void SubFileHash(int p, vector<array<int, 2>> & temp1_counter, vector<array<int,2>> & temp2_counter, Table* t_res, int joini1, int joini2, int maxrowsi1, int maxrowsi2){

    //Choose (bigger) relation to hash; setting up variables accordingly
    int which_hash = 2;
    int hashJoinIndex = joini2, probeJoinIndex = joini1;
    string hashfile = parsedQuery.joinSecondRelationName+"_"+to_string(p);
    if (temp1_counter[p][1] > temp2_counter[p][1]) {
        which_hash = 1;
        hashfile = parsedQuery.joinFirstRelationName+"_"+to_string(p);
        hashJoinIndex = joini1;
        probeJoinIndex = joini2;
    }

    // Calculating iterations and blocks
    int totalBlocks = (which_hash == 1) ? temp1_counter[p][1] : temp2_counter[p][1];
    int probeTotalBlocks = (which_hash == 1) ? temp2_counter[p][1] : temp1_counter[p][1];
    string probeFile = (which_hash == 1) ? parsedQuery.joinSecondRelationName : parsedQuery.joinFirstRelationName;
    probeFile += "_"+to_string(p); 
    int numSubfiles = (totalBlocks + PARTITION_COUNT - 1) / PARTITION_COUNT;
    // cout << "numSubfiles: " << numSubfiles << endl;

    // Iterate through subfile groups of hash relation
    for (int subfileGroup = 0; subfileGroup < numSubfiles; subfileGroup++) {
        
        //create inMemory Hash for all blocks of a subfile
        // cout << "subfile count" << subfileGroup << endl;
        unordered_map<int, vector<vector<int>>> inMemoryHash;
        // Load up to PARTITION_COUNT blocks from the hash relation to form one subfile
        int blocksLoaded = 0, rows_loaded=0;
        int blockIndex = subfileGroup * PARTITION_COUNT;
        int max_rows= which_hash == 1 ? maxrowsi1 : maxrowsi2;

        Cursor ca = Cursor(hashfile, blockIndex,  totalBlocks);
        vector<int> rowa = ca.getNextHashrow();
        while (blocksLoaded < PARTITION_COUNT && blockIndex < totalBlocks) {
            // cout << "Debug: Loading block " << blockIndex << " of partition " << p << " of relation " << ((which_hash == 1) ? parsedQuery.joinFirstRelationName : parsedQuery.joinSecondRelationName) << " into memory." << endl;
            // cout << "Current Hash page:" << ca.pageIndex << endl;
            while (!rowa.empty()) {
                rows_loaded++;
                int key = rowa[hashJoinIndex];
                inMemoryHash[key].push_back(rowa);
                if(rows_loaded>=max_rows){
                    rows_loaded=0;
                    rowa = ca.getNextHashrow();
                    break;
                }
                else if(ca.pageIndex != blockIndex){
                    rows_loaded=0;
                    break;
                }
                else{
                    rowa = ca.getNextHashrow();
                }
            }
            blocksLoaded++;
            blockIndex++;
            // cout << "blockIndex: " << blockIndex << " blockLoaded: << " << blocksLoaded << endl;
        }
        

        // Probe with the entire probe relation's partition (one block at a time) for subfile specifically
        Cursor cb = Cursor(probeFile, 0, probeTotalBlocks);
        vector<int> rowb = cb.getNextHashrow();
        while (!rowb.empty()) {
            int key = rowb[probeJoinIndex];
            if (inMemoryHash.find(key) != inMemoryHash.end()) {
                // Found a matching key, join and write the result
                for (const auto &hashRow : inMemoryHash[key]) {
                    vector<int> joinedRow;
                    joinedRow.reserve(t_res->columnCount);

                    // Join rows from hash relation and probe relation
                    if (which_hash == 1) {
                        joinedRow.insert(joinedRow.end(), hashRow.begin(), hashRow.end());
                        joinedRow.insert(joinedRow.end(), rowb.begin(), rowb.end());
                    } else {
                        joinedRow.insert(joinedRow.end(), rowb.begin(), rowb.end());
                        joinedRow.insert(joinedRow.end(), hashRow.begin(), hashRow.end());
                    }
                    t_res->writeRow<int>(joinedRow);
                }
            }
            rowb = cb.getNextHashrow();
        }
    }
}

void StandardHash(int p, vector<array<int, 2>> & temp1_counter, vector<array<int,2>> & temp2_counter, Table* t_res, int joini1, int joini2){
    // NORMAL JOIN 

    //When smaller relation R fits in M-1 buffers
    int which_hash=2;
    string hashfile = parsedQuery.joinSecondRelationName + "_"+to_string(p); // + "_tempfile";
    if(temp1_counter[p][1]<temp2_counter[p][1]){
        which_hash=1;
        hashfile = parsedQuery.joinFirstRelationName +"_"+to_string(p);
    }
    int i = 0, total_blocks = which_hash == 1 ? temp1_counter[p][1] : temp2_counter[p][1];
    
    // Using IN-MEMORY HASH, for further efficiency
    vector<int> rowa;
    unordered_map<int, vector<vector<int>>> inMemoryHash;
    Cursor ca = Cursor(hashfile, i, total_blocks);
    rowa = ca.getNextHashrow();
    while (!rowa.empty()) {
        int key = rowa[which_hash == 1 ? joini1 : joini2];
        inMemoryHash[key].push_back(rowa);
        rowa = ca.getNextHashrow();
    }

    //PROBING PHASE 

    // Setting up variables
    int probeRelation = (which_hash == 1) ? 2 : 1;
    string probeFile = (probeRelation == 1) ? parsedQuery.joinFirstRelationName : parsedQuery.joinSecondRelationName;
    probeFile += "_"+to_string(p);
    int probeBlockCnt = (which_hash == 1) ? temp2_counter[p][1] : temp1_counter[p][1];

    // Probing the hash table with the other relation
    Cursor cb = Cursor(probeFile, 0, probeBlockCnt);
    vector<int> rowb = cb.getNextHashrow();
    while (!rowb.empty()) {
        int key = rowb[probeRelation == 1 ? joini1 : joini2];
        if (inMemoryHash.find(key) != inMemoryHash.end()) {
            for (const auto &hashRow : inMemoryHash[key]) {
                vector<int> joinedRow;
                joinedRow.reserve(t_res->columnCount);

                if (which_hash == 1) {
                    joinedRow.insert(joinedRow.end(), hashRow.begin(), hashRow.end());
                    joinedRow.insert(joinedRow.end(), rowb.begin(), rowb.end());
                } else {
                    joinedRow.insert(joinedRow.end(), rowb.begin(), rowb.end());
                    joinedRow.insert(joinedRow.end(), hashRow.begin(), hashRow.end());
                }
                t_res->writeRow<int>(joinedRow);
            }
        }
        rowb = cb.getNextHashrow();
    }
}

void hashEquijoin(Table *t1, Table *t2, Table *t_res){
    //Getting join column indices
    int joini1 = t1->getColumnIndex(parsedQuery.joinFirstColumnName);
    int joini2 = t2->getColumnIndex(parsedQuery.joinSecondColumnName);
    
    //get maxRows size
    int maxrowsi1 = t1->maxRowsPerBlock;
    int maxrowsi2 = t2->maxRowsPerBlock;

    vector<array<int, 2>> temp1_counter(PARTITION_COUNT, {0, 0});
    vector<array<int, 2>> temp2_counter(PARTITION_COUNT, {0, 0});

    vector<vector<vector<int>>> store(PARTITION_COUNT);

    PartitionToHash(t1, joini1, maxrowsi1, temp1_counter, store, 1);
    store.clear();
    store.resize(PARTITION_COUNT);
    PartitionToHash(t2, joini2, maxrowsi2, temp2_counter, store, 2);
    store.clear();

    //Now we have iterations for each of the partition join
    for(int p=0; p<PARTITION_COUNT; p++){
        //Check which has the smaller subfile to hash
        int min_subpart=min(temp1_counter[p][1], temp2_counter[p][1]);
        if(!temp1_counter[p][1]||!temp2_counter[p][1])
            continue;
        if(min_subpart<=PARTITION_COUNT-1){ //Totalblocks >= R+2; Totalblocks=PARTITION_COUNT+1
            StandardHash(p, temp1_counter, temp2_counter, t_res, joini1, joini2);
        }
        else{
            SubFileHash(p, temp1_counter, temp2_counter, t_res, joini1, joini2, maxrowsi1, maxrowsi2);
        }
    }
   deleteTempHashFiles(parsedQuery.joinFirstRelationName, PARTITION_COUNT, temp1_counter);
   deleteTempHashFiles(parsedQuery.joinSecondRelationName, PARTITION_COUNT, temp2_counter);
}

void executeJOIN()
{
    logger.log("executeJOIN");

    // For a hash-based equijoin, we simply need to get the two tables.
    Table *t1 = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *t2 = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    // Prepare the output file.
    vector<string> columnNames;
    for (int i = 0; i < t1->columnCount; i++)
        columnNames.push_back(t1->columns[i]);
    for (int i = 0; i < (int)t2->columnCount; i++)
        columnNames.push_back(t2->columns[i]);

    Table *resultantTable = new Table(parsedQuery.joinResultRelationName, columnNames);
    hashEquijoin(t1, t2, resultantTable);
    resultantTable->blockify();
    tableCatalogue.insertTable(resultantTable);

    return;
}

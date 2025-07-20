#include "global.h"

string table_name;
vector<string> vec_attribute; 
vector<SortingStrategy> vec_strat;

struct CompareCursor {
    Table *table;
    vector<string> columnNames;
    vector<SortingStrategy> sortingStrategies;

    CompareCursor(Table *table, vector<string> columnNames,
                  vector<SortingStrategy> sortingStrategies)
        : table(table), columnNames(columnNames),
          sortingStrategies(sortingStrategies) {}

    bool operator()(Cursor &a, Cursor &b) {
        vector<int> rowA = a.getNextIfExists();
        vector<int> rowB = b.getNextIfExists();
        a.pagePointer--;
        b.pagePointer--;

        if (rowA.empty())
            return false;
        if (rowB.empty())
            return true;
        bool result =
            !table->isLess(rowA, rowB, columnNames, sortingStrategies);
        return result;
    }
};

/**
 * @brief Construct a new Table:: Table object
 *
 */
Table::Table()
{
    logger.log("Table::Table");
}

/**
 * @brief Construct a new Table:: Table object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param tableName 
 */
Table::Table(string tableName)
{
    logger.log("Table::Table");
    this->sourceFileName = "../data/" + tableName + ".csv";
    this->tableName = tableName;
}

/**
 * @brief Construct a new Table:: Table object used when an assignment command
 * is encountered. To create the table object both the table name and the
 * columns the table holds should be specified.
 *
 * @param tableName 
 * @param columns 
 */
Table::Table(string tableName, vector<string> columns)
{
    logger.log("Table::Table");
    this->sourceFileName = "../data/temp/" + tableName + ".csv";
    this->tableName = tableName;
    this->columns = columns;
    this->columnCount = columns.size();

    this->indexedCols = vector<bool>(this->columnCount, false);
    this->indexDirty = vector<bool>(this->columnCount, false);
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
    this->writeRow<string>(columns);
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates table
 * statistics.
 *
 * @return true if the table has been successfully loaded 
 * @return false if an error occurred 
 */
bool Table::load()
{
    logger.log("Table::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (this->extractColumnNames(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}

bool Table::loadIndex()
{
    logger.log("Table::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        this->columns.clear();
        if (this->extractColumnNames(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}

/**
 * @brief Function extracts column names from the header line of the .csv data
 * file. 
 *
 * @param line 
 * @return true if column names successfully extracted (i.e. no column name
 * repeats)
 * @return false otherwise
 */
bool Table::extractColumnNames(string firstLine)
{
    logger.log("Table::extractColumnNames");
    unordered_set<string> columnNames;
    string word;
    stringstream s(firstLine);
    while (getline(s, word, ','))
    {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        if (columnNames.count(word))
            return false;
        columnNames.insert(word);
        this->columns.emplace_back(word);
    }
    this->columnCount = this->columns.size();
    this->indexedCols = vector<bool>(this->columnCount, false);
    this->indexDirty = vector<bool>(this->columnCount, false);
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
    return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Table::blockify()
{
    logger.log("Table::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<int> row(this->columnCount, 0);
    vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
    int pageCounter = 0;
    unordered_set<int> dummy;
    dummy.clear();
    this->distinctValuesInColumns.assign(this->columnCount, dummy);
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    getline(fin, line);
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (!getline(s, word, ','))
                return false;
            row[columnCounter] = stoi(word);
            rowsInPage[pageCounter][columnCounter] = row[columnCounter];
        }
        pageCounter++;
        this->updateStatistics(row);
        if (pageCounter == this->maxRowsPerBlock)
        {
            bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter);
            this->blockCount++;
            this->rowsPerBlockCount.emplace_back(pageCounter);
            pageCounter = 0;
        }
    }
    if (pageCounter)
    {
        bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter);
        this->blockCount++;
        this->rowsPerBlockCount.emplace_back(pageCounter);
        pageCounter = 0;
    }

    if (this->rowCount == 0)
        return false;
    this->distinctValuesInColumns.clear();
    return true;
}

/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row 
 */
void Table::updateStatistics(vector<int> row)
{
    this->rowCount++;
    if (this->distinctValuesInColumns.size() == 0)
        return;
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
        {
            this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
            this->distinctValuesPerColumnCount[columnCounter]++;
        }
    }
}

/**
 * @brief Checks if the given column is present in this table.
 *
 * @param columnName 
 * @return true 
 * @return false 
 */
bool Table::isColumn(string columnName)
{
    logger.log("Table::isColumn");
    for (auto col : this->columns)
    {
        if (col == columnName)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Renames the column indicated by fromColumnName to toColumnName. It is
 * assumed that checks such as the existence of fromColumnName and the non prior
 * existence of toColumnName are done.
 *
 * @param fromColumnName 
 * @param toColumnName 
 */
void Table::renameColumn(string fromColumnName, string toColumnName)
{
    logger.log("Table::renameColumn");
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (columns[columnCounter] == fromColumnName)
        {
            columns[columnCounter] = toColumnName;
            break;
        }
    }
    return;
}

/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Table::print()
{
    logger.log("Table::print");
    uint count = min((long long)PRINT_COUNT, this->rowCount);

    //print headings
    this->writeRow(this->columns, cout);
    NO_OF_BLOCK_READ++;
    Table* T = tableCatalogue.getTable(this->tableName);
    if(T->blockCount == 0) {
        return;
    }
    Cursor cursor(this->tableName, 0);


    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        row = cursor.getNext();
        if(row.empty()) {
            count++;
            continue;
        }
        this->writeRow(row, cout);
    }
    printRowCount(this->rowCount);
}



/**
 * @brief This function returns one row of the table using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor 
 * @return vector<int> 
 */
void Table::getNextPage(Cursor *cursor)
{
    logger.log("Table::getNext");

        if (cursor->pageIndex < this->blockCount - 1)
        {
            cursor->nextPage(cursor->pageIndex+1);
        }
        NO_OF_BLOCK_READ++;
}



/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Table::makePermanent()
{
    logger.log("Table::makePermanent");
    if(!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->tableName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    //print headings
    this->writeRow(this->columns, fout);
    
    Cursor cursor(this->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, fout);
    }
    fout.close();
}

/**
 * @brief Function to check if table is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Table::isPermanent()
{
    logger.log("Table::isPermanent");
    if (this->sourceFileName == "../data/" + this->tableName + ".csv")
    return true;
    return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Table::unload(){
    logger.log("Table::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->tableName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this table
 * 
 * @return Cursor 
 */
Cursor Table::getCursor()
{
    logger.log("Table::getCursor");
    Cursor cursor(this->tableName, 0);
    return cursor;
}
/**
 * @brief Function that returns the index of column indicated by columnName
 * 
 * @param columnName 
 * @return int 
 */
int Table::getColumnIndex(string columnName)
{
    logger.log("Table::getColumnIndex");
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (this->columns[columnCounter] == columnName)
            return columnCounter;
    }
}


bool Table::isLess(vector<int> &a, vector<int> &b, vector<string> columnNames,
                   vector<SortingStrategy> sortingStrategies) {
    vector<int> columnIndices;
    for (const string &columnName : columnNames) {
        columnIndices.push_back(getColumnIndex(columnName));
    }

    for (int i = 0; i < columnIndices.size(); i++) {
        if (a[columnIndices[i]] < b[columnIndices[i]]) {
            return sortingStrategies[i] == ASC;
        } else if (a[columnIndices[i]] > b[columnIndices[i]]) {
            return sortingStrategies[i] == DESC;
        }
    }
    return false;
}

void renamePage(string pageName, string modifiedPageName) {
    logger.log("renamePage");
    // cout << "Renaming " << pageName << " to " << modifiedPageName << endl;
    pageName = "../data/temp/" + pageName;
    modifiedPageName = "../data/temp/" + modifiedPageName;
    if (rename(pageName.c_str(), modifiedPageName.c_str()) != 0) {
        perror("Error renaming file");
    }
}

void Table::sort(vector<string> columnNames,
                 vector<SortingStrategy> sortingStrategies) {
    logger.log("Table::sort");
    bufferManager.clear();
    // Sort Phase 1 : Sort each block Individually
    int nB = BLOCK_COUNT_P2;
    int dM = min(nB - 1, (int)this->blockCount);
    int blocks = this->blockCount;
    for (int i = 0; i < blocks; i++) {
        NO_OF_BLOCK_READ++;
        Page page = bufferManager.getPage(this->tableName, i);
        // Resize is required because last page always get's generated of full
        // size
        page.resize(this->rowsPerBlockCount[i]);
        page.sortPage(columnNames, sortingStrategies);
        page.writePage();
        bufferManager.removePage(this->tableName, i);
    }
    bufferManager.clear();
    
    
    // Merge Phase..
    int subFiles = blocks;
    int initialBlocks = blocks;
    int gap = 1;
    int cnt = 0;
    while (subFiles > 1) {
        priority_queue<Cursor, vector<Cursor>, CompareCursor> pq(
            CompareCursor(this, columnNames, sortingStrategies));
        int subFilesLeftMerge = subFiles;
        for(int i = 0; subFilesLeftMerge >= 1; i += gap) {
            // cout << "Adding file " << i << " to priority queue" << endl;
            pq.push(Cursor(this->tableName, i));
            NO_OF_BLOCK_READ++;
            if(pq.size() == min(dM, subFilesLeftMerge)) {
                // do the merge
                subFilesLeftMerge -= pq.size();
                // cout << "Merging " << pq.size() << " files" << endl;

                vector<vector<int>> rows;
                int rowCount = 0;

                while(!pq.empty()) {
                    Cursor cursor = pq.top();
                    pq.pop();
                    vector<int> row = cursor.getNextIfExists();
                    if(row.empty()) {
                        // Cursor ended...
                        if((cursor.pageIndex + 1) % (gap) != 0) {
                            bufferManager.removePage(cursor.tableName, cursor.pageIndex);
                            // Add the cursor back with next page
                            if(cursor.pageIndex + 1 < initialBlocks) {
                                NO_OF_BLOCK_READ++;
                                cursor = Cursor(this->tableName, cursor.pageIndex + 1);
                                pq.push(cursor);
                                // cout << "Adding cursor : " << cursor.pageIndex << endl;
                                // logger.log("Adding cursor : " + to_string(cursor.pageIndex));
                            }
                        }
                        continue;
                    }
                    if(!row.empty()) {
                        cnt += 1;
                        pq.push(cursor);
                        rows.push_back(row);
                        rowCount++;
                        if(rowCount == this->maxRowsPerBlock) {
                            bufferManager.writePage(this->tableName, blocks, rows, rowCount);
                            blocks++;
                            rows.clear();
                            rowCount = 0;
                        }
                    }
                }
                if(rowCount > 0) {
                    bufferManager.writePage(this->tableName, blocks, rows, rowCount);
                    blocks++;
                    rows.clear();
                    rowCount = 0;
                }
            }
        }
        for(int i = 0; i < blocks / 2; i++) {
            bufferManager.deleteFile(this->tableName, i);
        }
        for(int i = blocks / 2; i < blocks; i++) {
            renamePage(this->tableName + "_Page" + to_string(i), this->tableName + "_Page" + to_string(i - blocks / 2));
        }
        gap += (dM - 1);
        subFiles = (subFiles + dM - 1) / dM;
        blocks = blocks - blocks / 2;
    }
    bufferManager.clear();
}

void Table::appendRowToTable(vector<int>& newRow){
    int last = this->blockCount - 1;

    // this->writeRow(newRow); //TBD: If needed for future operations

    if (last >= 0 && this->rowsPerBlockCount[last] < this->maxRowsPerBlock){
        vector<vector<int>> rows={}; //TBD: add this to assumption. That some memory may be exceeded
        Cursor cursor(this->tableName, last);
        vector<int> row = cursor.getNextIfExists();
        while(!row.empty()){
            rows.push_back(row);
            row = cursor.getNextIfExists();
        }
        rows.push_back(newRow);
        this->updateStatistics(newRow);
        // this->blockCount++;
        this->rowsPerBlockCount[last] = this->rowsPerBlockCount[last] + 1;
        // cout << "block no: " << last << " row count: " << this->rowsPerBlockCount[last] << endl; 
        bufferManager.writePage(this->tableName, last, rows, this->rowsPerBlockCount[last]);
    }
    else{
        vector<vector<int>> rows={};
        rows.push_back(newRow);
        this->updateStatistics(newRow);
        this->blockCount++;
        this->rowsPerBlockCount.push_back(1);
        bufferManager.writePage(this->tableName, this->blockCount - 1, rows, rows.size());
    }

    this->markAllColsIndexDirty();
}

void Table::setasIndexTable() {
    logger.log("Table::setasIndexTable");
    this->isIndexTable = true;
}

bool Table::buildSecondaryIndex(const string &col) {
    const string base   = this->tableName;
    const string idxName = "idx_" + base + "_" + col;
    // const string idxName= T->indexTableName(col);

    if(!this->indexed) {
        this->indexed = true;
    }

    // 1A) check if the index already exists
   if (this->hasIndexOn(col) || tableCatalogue.isTable(idxName)) {
        tableCatalogue.deleteTable(idxName);
    }

    // 1) make a fresh Table object for idxName
    Table *I = new Table(idxName, vector<string>{col,"page","offset"});
    I->setasIndexTable();
    tableCatalogue.insertTable(I);
    

    // 2) stream (key,page,offset) from T into I’s CSV
    int colIdx = this->getColumnIndex(col);
    for (uint pg = 0; pg < this->blockCount; ++pg) {
      Page P = bufferManager.getPage(base, pg);
      int rows = this->rowsPerBlockCount[pg];
    //   cout << "rows: " << rows << endl;
      for (int r = 0; r < rows; ++r) {
        auto row = P.getRow(r);
        // cout << "r = row_index = " << r << endl;
        int key = row[colIdx];
        I->writeRow(vector<int>{key,(int)pg,r});
      }
    }

    if (!I->loadIndex()) {
        cout << "ERROR: failed to blockify index table\n";
        return false;
    }

    I->sort({ col }, { ASC }); //prexisting external sort on I

    // 4) tell T that its index on col is now built & clean
    this->markIndexRebuilt(col);

    // 5) Mark the base table as indexed
    this->indexed            = true;
    this->indexedColumn      = col;
    this->indexingStrategy   = HASH;      // Redundant

    return true;
}

void Table::registerIndexOn(const string &col) {
    int idx = this->getColumnIndex(col);
    this->indexedCols[idx] = true;
    this->indexDirty[idx] = false;
}
/**
 * @brief Function to mark the index as dirty. This is used when the table is
 * modified and the index needs to be updated.
 *
 * @param col 
 */
void Table::markIndexDirty(string &col) {
    int idx = this->getColumnIndex(col);
    this->indexDirty[idx] = true;
}

void Table::markIndexRebuilt(const string& col) {
    int idx = this->getColumnIndex(col);
    this->indexedCols[idx] = true;
    this->indexDirty[idx] = false;
}

void Table::markAllColsIndexDirty(){
    for(int i=0; i<this->columnCount; i++){
        this->indexDirty[i] = true;
    }
}

void Table::markwholeIndexRebuilt(){
    if(this->indexedCols.size() != this->columnCount)
        this->indexedCols = vector<bool>(this->columnCount, true);
    else{
        for(int i=0; i<this->columnCount; i++){
            this->indexedCols[i] = true;
        }
    }
    if(this->indexDirty.size() != this->columnCount)
        this->indexDirty = vector<bool>(this->columnCount, false);
    else{
        for(int i=0; i<this->columnCount; i++){
            this->indexDirty[i] = false;
        }
    }
}

// true if we have ever built an index on `col`
bool Table::hasIndexOn(const string &col) {
    int idx = this->getColumnIndex(col);
    return this->indexedCols[idx];
}

// true if that index is currently dirty
bool Table::isIndexDirty(const string &col) {
    int idx = this->getColumnIndex(col);
    if (this->indexedCols[idx] == true){
        return this->indexDirty[idx];
    }
    else
        return true;
}

    
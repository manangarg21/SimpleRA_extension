#include "global.h"
/**
 * @brief Construct a new Page object. Never used as part of the code
 *
 */
Page::Page()
{
    this->pageName = "";
    this->tableName = "";
    this->pageIndex = -1;
    this->rowCount = 0;
    this->columnCount = 0;
    this->rows.clear();
}

/**
 * @brief Construct a new Page:: Page object given the table name and page
 * index. When tables are loaded they are broken up into blocks of BLOCK_SIZE
 * and each block is stored in a different file named
 * "<tablename>_Page<pageindex>". For example, If the Page being loaded is of
 * table "R" and the pageIndex is 2 then the file name is "R_Page2". The page
 * loads the rows (or tuples) into a vector of rows (where each row is a vector
 * of integers).
 *
 * @param tableName 
 * @param pageIndex 
 */
Page::Page(string tableName, int pageIndex)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);
    Table table = *tableCatalogue.getTable(tableName);
    this->columnCount = table.columnCount;
    uint maxRowCount = table.maxRowsPerBlock;
    vector<int> row(columnCount, 0);
    this->rows.assign(maxRowCount, row);

    ifstream fin(pageName, ios::in);
    this->rowCount = table.rowsPerBlockCount[pageIndex];
    int number;
    for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
        {
            fin >> number;
            this->rows[rowCounter][columnCounter] = number;
        }
    }
    fin.close();
}

/**
 * @brief Get row from page indexed by rowIndex
 * 
 * @param rowIndex 
 * @return vector<int> 
 * Note: if this is a matrix page, rowIndex should be 0 for valid data. 
 */
vector<int> Page::getRow(int rowIndex)
{
    logger.log("Page::getRow");
    vector<int> result;
    result.clear();
    if (rowIndex >= this->rowCount)
        return result;
    return this->rows[rowIndex];
}

Page::Page(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->rows = rows;
    this->rowCount = rowCount;
    if (!rows.empty()) {
        this->columnCount = rows[0].size();
    } else {
        this->columnCount = 0;  // or throw an exception, depending on your logic
    }    
    this->pageName = "../data/temp/"+this->tableName + "_Page" + to_string(pageIndex);
}

/**
 * @brief writes current page contents to file.
 * Note: If it's a matrix page, rowCount=1, so we write a single line of `columnCount`
 * numbers. If it's a table page with multiple rows, we write multiple lines.
 */
void Page::writePage()
{
    logger.log("Page::writePage");
    NO_OF_BLOCK_WRITTEN++;
    ofstream fout(this->pageName, ios::trunc);
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (columnCounter != 0)
                fout << " ";
            fout << this->rows[rowCounter][columnCounter];
        }
        fout << endl;
    }
    fout.close();
}


/**
 * @brief NEW: Constructor for matrix pages (CREATE in memory).
 * 
 * We store all `data` as a single row. rowCount=1, columnCount=data.size().
 */
Page::Page(string matrixName, int pageIndex, vector<int> &data, bool isMatrix) {
    logger.log("Page::Page (matrix mode, from in-memory vector)");
    this->tableName = matrixName; // reuse the same field for matrixName
    this->pageIndex = pageIndex;
    this->isMatrix = isMatrix; // should be true in this usage
    this->pageName = "../data/temp/" + matrixName + "_Page" + to_string(pageIndex);

    // Single row
    this->rowCount = 1;
    this->columnCount = data.size();
    this->rows.resize(1);
    this->rows[0] = data;  // put entire vector as one row
}

/**
 * @brief NEW: Constructor for matrix pages (LOAD from disk).
 * 
 * We read all integers in the file into a single row: row[0].
 */
Page::Page(string matrixName, int pageIndex, bool isMatrixReading) {
    logger.log("Page::Page (matrix mode, read from disk)");
    this->tableName = matrixName;
    this->pageIndex = pageIndex;
    this->isMatrix = true; // forced
    this->pageName = "../data/temp/" + matrixName + "_Page" + to_string(pageIndex);

    ifstream fin(this->pageName, ios::in);
    if (!fin.is_open()) {
        // If file not found or cannot open
        this->rowCount = 0;
        this->columnCount = 0;
        return;
    }

    vector<int> singleRow;
    int number;
    while (fin >> number) {
        singleRow.push_back(number);
    }
    fin.close();

    this->rowCount = 1;
    this->columnCount = singleRow.size();
    this->rows.resize(1);
    this->rows[0] = singleRow;
}

void Page::sortPage(vector<string> columnNames,
                    vector<SortingStrategy> sortingStrategies) {
    logger.log("Page::sortPage");
    vector<int> columnIndices;
    for (const string &columnName : columnNames) {
        columnIndices.push_back(tableCatalogue.getTable(this->tableName)
                                    ->getColumnIndex(columnName));
    }

    sort(this->rows.begin(), this->rows.end(),
         [columnIndices, sortingStrategies](const vector<int> &a,
                                            const vector<int> &b) {
             for (int i = 0; i < columnIndices.size(); i++) {
                 if (a[columnIndices[i]] < b[columnIndices[i]]) {
                     return sortingStrategies[i] == ASC;
                 } else if (a[columnIndices[i]] > b[columnIndices[i]]) {
                     return sortingStrategies[i] == DESC;
                 }
             }
             return false;
         });
}

void Page::resize(int rowCount) {
    logger.log("Page::resize");
    this->rows.resize(rowCount);
    this->rowCount = rowCount;
}

Page Page::Hashpage(string Hashfile, int pageIndex)
{
    Page hashPage;
    hashPage.tableName = "";
    hashPage.pageIndex = pageIndex; // Default index for hash pages
    hashPage.pageName = "../data/temp/" + Hashfile + "_Hash_Page" + to_string(pageIndex);
    string tableName = Hashfile.substr(0, Hashfile.rfind('_'));
    // cout << "Table Name for Page: " << tableName << endl;
    Table table = *tableCatalogue.getTable(tableName);
    hashPage.columnCount = table.columnCount;
    
    ifstream fin(pageName, ios::in);
    this->rowCount = 0;
    int number;
    bool endloop=0;
    vector<int> row(columnCount, 0);
    // cout << "Ready to get table" << endl;
    while (fin.peek() != EOF && fin.good())
    {
        for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
        {
            if (!(fin >> number)) {
                endloop=1;
                break;
            }
            row[columnCounter] = number;
        }
        if(endloop)
            break;
        hashPage.rows.push_back(row);
        this->rowCount++;
    }
    // cout << "Hashfile Name: " << Hashfile << ", Hash page rowCount: " << this->rowCount << endl;
    //TBD : TO BE DELETED
    
    fin.close();
    
    hashPage.isMatrix = false; // Hash pages are not matrices
    return hashPage;
}

void Page::deleteRow(int rowIndex) {
    logger.log("Page::deleteRow");
    if (rowIndex >= this->rowCount) {
        cout << "Row index out of bounds" << endl;
        return;
    }
    this->rows.erase(this->rows.begin() + rowIndex);
    this->rowCount--;
    this->writePage();
}

void Page::updateRow(int rowIndex, int updateColumnIndex, int updateValue){
    logger.log("Page::updateRow");

    if (rowIndex >= this->rowCount || updateColumnIndex >= this->columnCount) {
        cout << "Row or column index out of bounds" << endl;
        return;
    }
    this->rows[rowIndex][updateColumnIndex] = updateValue;
    this->writePage();
}
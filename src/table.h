#pragma once
#include "cursor.h"

enum IndexingStrategy
{
    BTREE,
    HASH,
    NOTHING
};

// enum SortingStrategy
// {
//     ASC,
//     DESC,
//     NO_SORT_CLAUSE
// };

/**
 * @brief The Table class holds all information related to a loaded table. It
 * also implements methods that interact with the parsers, executors, cursors
 * and the buffer manager. There are typically 2 ways a table object gets
 * created through the course of the workflow - the first is by using the LOAD
 * command and the second is to use assignment statements (SELECT, PROJECT,
 * JOIN, SORT, CROSS and DISTINCT). 
 *
 */
class Table
{
    vector<unordered_set<int>> distinctValuesInColumns;
    bool isIndexTable = false;

public:
    string sourceFileName = "";
    string tableName = "";
    vector<string> columns;
    vector<uint> distinctValuesPerColumnCount;
    uint columnCount = 0;
    long long int rowCount = 0;
    uint blockCount = 0;
    uint maxRowsPerBlock = 0;
    vector<uint> rowsPerBlockCount;
    bool indexed = false;
    string indexedColumn = "";
    IndexingStrategy indexingStrategy = NOTHING;
    
    bool extractColumnNames(string firstLine);
    bool blockify();
    void updateStatistics(vector<int> row);
    Table();
    Table(string tableName);
    Table(string tableName, vector<string> columns);
    bool load();
    bool loadIndex();
    bool isColumn(string columnName);
    void renameColumn(string fromColumnName, string toColumnName);
    void print();
    void makePermanent();
    bool isPermanent();
    void getNextPage(Cursor *cursor);
    Cursor getCursor();
    int getColumnIndex(string columnName);
    void unload();
    bool isLess(vector<int> &a, vector<int> &b, vector<string> columnNames,
        vector<SortingStrategy> sortingStrategies);
    void sort(vector<string> columnNames, vector<SortingStrategy> sortingStrategies);

    // unordered_map<int, vector<pair<int,int>>> hashIndex;   // key → {page,row}
    // map<int,vector<pair<int,int>>> secondaryIndex;
    vector<bool> indexedCols;
    vector<bool> indexDirty; //or map<int,bool> SecondaryIndex;
    void setasIndexTable();
    bool hasIndexOn(const string &col); // returns true if index has prev built for that attribute
    bool buildSecondaryIndex(const string &col);
    void registerIndexOn(const string &col);
    void markIndexRebuilt(const string& col);
    void markAllColsIndexDirty();
    void markwholeIndexRebuilt();
    void markIndexDirty(string &col);
    bool isIndexDirty(const string &col);
    
    void appendRowToTable(vector<int> &newRow);
    string secondaryIndexName;




    /**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
template <typename T>
void writeRow(vector<T> row, ostream &fout)
{
    logger.log("Table::printRow");
    for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
    {
        if (columnCounter != 0)
            fout << ", ";
        fout << row[columnCounter];
    }
    fout << endl;
}

/**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
template <typename T>
void writeRow(vector<T> row)
{
    logger.log("Table::printRow");
    ofstream fout(this->sourceFileName, ios::app);
    this->writeRow(row, fout);
    fout.close();
}
};
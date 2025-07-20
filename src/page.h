#include "logger.h"
#pragma once
/**
 * @brief The Page object is the main memory representation of a physical page
 * (equivalent to a block). The page class and the page.h header file are at the
 * bottom of the dependency tree when compiling files.
 *<p>
 * Do NOT modify the Page class. If you find that modifications
 * are necessary, you may do so by posting the change you want to make on Moodle
 * or Teams with justification and gaining approval from the TAs.
 *</p>
 */
enum SortingStrategy { ASC, DESC, NO_SORT_CLAUSE };

class Page {

    string tableName;
    string pageIndex;
    int columnCount;
    int rowCount;
    vector<vector<int>> rows;

    bool isMatrix = false;

  public:
    string pageName = "";
    Page();
    Page(string tableName, int pageIndex);
    Page(string tableName, int pageIndex, vector<vector<int>> rows,
         int rowCount);
    vector<int> getRow(int rowIndex);
    void writePage();

    void sortPage(vector<string> columnNames,
                  vector<SortingStrategy> sortingStrategies);

    void resize(int rowCount);
    // NEW: For MATRIX usage: create from an in-memory 1D vector
    Page(string matrixName, int pageIndex, vector<int> &data, bool isMatrix);
    // NEW: For MATRIX usage: read from disk into a single row
    Page(string matrixName, int pageIndex, bool isMatrixReading);

    Page Hashpage(string Hashfile, int pageIndex);

    int getRowCount() const { return this->rowCount; }
    int getColumnCount() const { return this->columnCount; }

    void deleteRow(int rowIndex);
    void updateRow(int rowIndex, int updateColumnIndex, int updateValue);
};
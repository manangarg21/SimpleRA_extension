#include"bufferManager.h"
#pragma once
/**
 * @brief The cursor is an important component of the system. To read from a
 * table, you need to initialize a cursor. The cursor reads rows from a page one
 * at a time.
 *
 */
class Cursor{
    public:
    Page page;
    int pageIndex;
    string tableName;
    int pagePointer;

    bool matrixMode;           
    // Page matrixPage; //EDIT: not sure if correct
    // int matrixPagePointer;

    //EDIT: hash pages
    int totalhashPages;

    public:
    Cursor(string tableName, int pageIndex);
    vector<int> getNext();
    vector<int> getNextIfExists();
    vector<int> getNextMatrix();
    void nextPage(int pageIndex);

    Cursor(string matrixName, int pageIndex, bool isMatrix);
    void nextMatrixPage(int pageIndex);
    int getElementAtMatrixOffset(int offset);

    Cursor(string hashedTableName, int pageIndex, int total);
    vector<int> getNextHashrow();
    void nextHashPage(int pageIndex);

};
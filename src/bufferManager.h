#include"page.h"
#pragma once
/**
 * @brief The BufferManager is responsible for reading pages to the main memory.
 * Recall that large files are broken and stored as blocks in the hard disk. The
 * minimum amount of memory that can be read from the disk is a block whose size
 * is indicated by BLOCK_SIZE. within this system we simulate blocks by
 * splitting and storing the file as multiple files each of one BLOCK_SIZE,
 * although this isn't traditionally how it's done. You can alternatively just
 * random access to the point where a block begins within the same
 * file. In this system we assume that the the sizes of blocks and pages are the
 * same. 
 * 
 * <p>
 * The buffer can hold multiple pages quantified by BLOCK_COUNT. The
 * buffer manager follows the FIFO replacement policy i.e. the first block to be
 * read in is replaced by the new incoming block. This replacement policy should
 * be transparent to the executors i.e. the executor should not know if a block
 * was previously present in the buffer or was read in from the disk. 
 * </p>
 *
 */
class BufferManager{

    deque<Page> pages; 
    bool inPool(string pageName);
    Page getFromPool(string pageName);
    Page insertIntoPool(string tableName, int pageIndex);

    //for block data handling
    vector<int> tempblock0;
    vector<int> tempblock1;

    public:
    
    BufferManager();
    Page getPage(string tableName, int pageIndex);
    void writePage(string pageName, vector<vector<int>> rows);
    void renamePage(string pageName, string modifiedPageName);
    void deleteFile(string tableName, int pageIndex);
    void deleteFile(string fileName);
    void writePage(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount);

    // OLD functions
    // void writeMatrixPage(string matrixName, int pageIndex, vector<int> data, int elementCount);
    // vector<int> getMatrixPage(string matrixName, int pageIndex);

    // For MATRIX usage: now returns Page instead of vector<int>
    Page getMatrixPage(std::string matrixName, int pageIndex);

    void writeMatrixPage(std::string matrixName, int pageIndex, vector<int> &data);

    //TEMP BLOCK HANDLING
    vector<int>& loadTempBlock(vector<int> data, int tempBuffIndex);

    void clear();

    void removePage(string tableName, int pageIndex);

    // HASH FUNCTION ADDITIONS
    Page getHashPage(string tableName, int pageIndex);
    void writeHashPage(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount);
    void deleteHashFile(string HashName, int pageIndex);
    Page insertHashIntoPool(string tableName, int pageIndex);
    int getNextHashPageId(const string &table,
        const string &col, int bucket);
};
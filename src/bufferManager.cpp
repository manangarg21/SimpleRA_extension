#include "global.h"

using namespace std;
BufferManager::BufferManager()
{
    logger.log("BufferManager::BufferManager");
}

/**
 * @brief Function called to read a page from the buffer manager. If the page is
 * not present in the pool, the page is read and then inserted into the pool.
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page 
 */
Page BufferManager::getPage(string tableName, int pageIndex)
{
    logger.log("BufferManager::getPage");
    string pageName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    if (this->inPool(pageName))
        return this->getFromPool(pageName);
    else
        return this->insertIntoPool(tableName, pageIndex);
}

/**
 * @brief Checks to see if a page exists in the pool
 *
 * @param pageName 
 * @return true 
 * @return false 
 */
bool BufferManager::inPool(string pageName)
{
    logger.log("BufferManager::inPool");
    for (auto page : this->pages)
    {
        if (pageName == page.pageName)
            return true;
    }
    return false;
}

/**
 * @brief If the page is present in the pool, then this function returns the
 * page. Note that this function will fail if the page is not present in the
 * pool.
 *
 * @param pageName 
 * @return Page 
 */
Page BufferManager::getFromPool(string pageName)
{
    logger.log("BufferManager::getFromPool");
    for (auto page : this->pages)
        if (pageName == page.pageName)
            return page;
}

/**
 * @brief Inserts page indicated by tableName and pageIndex into pool. If the
 * pool is full, the pool ejects the oldest inserted page from the pool and adds
 * the current page at the end. It naturally follows a queue data structure. 
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page 
 */
Page BufferManager::insertIntoPool(string tableName, int pageIndex)
{
    logger.log("BufferManager::insertIntoPool");
    Page page(tableName, pageIndex);
    if (this->pages.size() >= BLOCK_COUNT)
        pages.pop_front();
    pages.push_back(page);
    return page;
}


/**
 * @brief The buffer manager is also responsible for writing pages. This is
 * called when new tables are created using assignment statements.
 *
 * @param tableName 
 * @param pageIndex 
 * @param rows 
 * @param rowCount 
 */
void BufferManager::writePage(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount)
{
    logger.log("BufferManager::writePage");
    Page page(tableName, pageIndex, rows, rowCount);

    //remove if there in buffer. New page written
    string pageName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    if(this->inPool(pageName)){
        for (auto it = this->pages.begin(); it != this->pages.end(); it++) {
            if (it->pageName == pageName) {
                this->pages.erase(it);
                break;
            }
        }
    }
    page.writePage();
}

/**
 * @brief Deletes file names fileName
 *
 * @param fileName 
 */
void BufferManager::deleteFile(string fileName)
{
    
    if (remove(fileName.c_str()))
        logger.log("BufferManager::deleteFile: Err");
        else logger.log("BufferManager::deleteFile: Success");
}

/**
 * @brief Overloaded function that calls deleteFile(fileName) by constructing
 * the fileName from the tableName and pageIndex.
 *
 * @param tableName 
 * @param pageIndex 
 */
void BufferManager::deleteFile(string tableName, int pageIndex)
{
    logger.log("BufferManager::deleteFile");
    string fileName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    this->deleteFile(fileName);
}

// void BufferManager::writeMatrixPage(string matrixName, int pageIndex, vector<int> data, int elementCount) {
//     logger.log("BufferManager::writeMatrixPage");
//     string pageName = "../data/temp/" + matrixName + "_Page" + to_string(pageIndex);
//     ofstream fout(pageName, ios::trunc);
//     if (!fout.is_open()) {
//         cout << "Error: Cannot open file " << pageName << " for writing." << endl;
//         return;
//     }
//     for (int i = 0; i < elementCount; i++) {
//         if (i != 0)
//             fout << " ";
//         fout << data[i];
//     }
//     fout << endl;
//     fout.flush();
//     fout.close();
// }

// vector<int> BufferManager::getMatrixPage(string matrixName, int pageIndex) {
//     logger.log("BufferManager::getMatrixPage");
//     string pageName = "../data/temp/" + matrixName + "_Page" + to_string(pageIndex);
//     // cout << pageName << endl;
//     vector<int> result;
//     ifstream fin(pageName, ios::in);
//     if (!fin.is_open()) {
//         cout << "Error: Cannot open matrix page " << pageName << endl;
//         return result;
//     }
//     int number;
//     while (fin >> number) {
//         result.push_back(number);
//     }
//     fin.close();
//     return result;
// }

/** 
 * @brief Return matrix page from the buffer if in pool, else load it from disk
 *        using our new matrix constructor.
 */
Page BufferManager::getMatrixPage(string matrixName, int pageIndex) {
    logger.log("BufferManager::getMatrixPage");
    string pageName = "../data/temp/" + matrixName + "_Page" + to_string(pageIndex);
    if (this->inPool(pageName)) {
        return this->getFromPool(pageName);
    } else {
        // Create a Page in matrix mode (read from disk as single row).
        Page newPage(matrixName, pageIndex, true /* isMatrixReading */);
        // Insert into pool
        if (this->pages.size() >= BLOCK_COUNT) {
            this->pages.pop_front();
        }
        this->pages.push_back(newPage);
        return newPage;
    }
}

/**
 * @brief Write matrix data (1D) to disk as a single row.
 */
void BufferManager::writeMatrixPage(string matrixName, int pageIndex,vector<int> &data) {
    // cout << "bruh" << endl;
    // for(int i = 0; i < data.size(); i++){
    //     cout << data[i] << " ";
    // }
    // cout << endl;
    logger.log("BufferManager::writeMatrixPage");
    Page page(matrixName, pageIndex, data, true);
    page.writePage();

    // Optionally insert into the buffer
    string pageName = page.pageName;
    for (auto it = this->pages.begin(); it != this->pages.end(); it++) {
        if (it->pageName == pageName) {
            this->pages.erase(it);
            break;
        }
    }
    // Then insert the updated page
    if (this->pages.size() >= BLOCK_COUNT)
        this->pages.pop_front();
    this->pages.push_back(page);
}

vector<int>& BufferManager::loadTempBlock(vector<int> data, int tempBuffIndex) {
    if(data.size()>BLOCK_SIZE*1000/sizeof(int)){
        cout<<"Error: Data size exceeds block size"<<endl;
    }
    if (tempBuffIndex == 0) {
        tempblock0 = move(data);
        return tempblock0;
    } else if (tempBuffIndex == 1) {
        tempblock1 = move(data);
        return tempblock1;
    } else {
        tempblock0 = move(data);
        return tempblock0;
    }
}

void BufferManager::clear() {
    this->pages.clear();
}

void BufferManager::removePage(string tableName, int pageIndex) {
    string pageName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    for (auto it = this->pages.begin(); it != this->pages.end(); it++) {
        if (it->pageName == pageName) {
            this->pages.erase(it);
            break;
        }
    }
}

Page BufferManager::getHashPage(string tableBucketName, int pageIndex)
{
    logger.log("BufferManager::getHashPage");
    string pageName = "../data/temp/"+ tableBucketName + "_Hash_Page" + to_string(pageIndex);
    // cout << "pageName: " << pageName << endl; 
    //Q: should we interface with buffer pool here?

    if (this->inPool(pageName))
        return this->getFromPool(pageName);
    else
        return this->insertHashIntoPool(tableBucketName, pageIndex);

}

void BufferManager::writeHashPage(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount)
{
    logger.log("BufferManager::writeHashPage");
    Page page(tableName+"_Hash", pageIndex, rows, rowCount);
    page.writePage();
    // cout << "page written" <<endl;
}

Page BufferManager::insertHashIntoPool(string tableName, int pageIndex)
{
    logger.log("BufferManager::insert(Hash)IntoPool");
    Page page= page.Hashpage(tableName, pageIndex);
    if (this->pages.size() >= BLOCK_COUNT) //Q: should this be changed?
        pages.pop_front();
    pages.push_back(page);
    return page;
}

void BufferManager::deleteHashFile(string HashName, int pageIndex)
{
    logger.log("BufferManager::deleteHashFile");
    string fileName = "../data/temp/"+ HashName + "_Hash_Page" + to_string(pageIndex);
    // cout << "Deleting Hash File: " << fileName << endl;
    this->deleteFile(fileName);
}

/* returns the id to be used for the next page of the bucket file */
int BufferManager::getNextHashPageId(const string &table,
    const string &col, int bucket)
{
string prefix = "../data/temp/" + table + "_" + col + "_B" +
to_string(bucket) + "_Page";
int id = 0;
struct stat st;
while (stat((prefix + to_string(id)).c_str(), &st) == 0) ++id;
return id;
}

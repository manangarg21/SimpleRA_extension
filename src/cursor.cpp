#include "global.h"

Cursor::Cursor(string tableName, int pageIndex) {
    logger.log("Cursor::Cursor");
    this->page = bufferManager.getPage(tableName, pageIndex);
    this->pagePointer = 0;
    this->tableName = tableName;
    this->pageIndex = pageIndex;
}

/**
 * @brief This function reads the next row from the page. The index of the
 * current row read from the page is indicated by the pagePointer(points to row
 * in page the cursor is pointing to).
 *
 * @return vector<int>
 */
vector<int> Cursor::getNext() {
    logger.log("Cursor::geNext");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    if (result.empty()) {
        tableCatalogue.getTable(this->tableName)->getNextPage(this);
        if (!this->pagePointer) {
            result = this->page.getRow(this->pagePointer);
            this->pagePointer++;
        }
    }
    return result;
}

/**
 * @brief This function reads the next row from the page. If there are no next
 * rows it returns an empty vector.
 *
 * @return vector<int>
 */
 vector<int> Cursor::getNextIfExists() {
    logger.log("Cursor::getNextIfExists");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    return result;
}

/**
 * @brief This function reads the next row from the page. The index of the
 * current row read from the page is indicated by the pagePointer(points to row
 * in page the cursor is pointing to).
 *
 * @return vector<int>
 */
vector<int> Cursor::getNextMatrix() {
    logger.log("Cursor::geNext (Matrix)");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    if (result.empty()) {
        matrixCatalogue.getMatrix(this->tableName)->getNextPage(this);
        if (!this->pagePointer) {
            result = this->page.getRow(this->pagePointer);
            this->pagePointer++;
        }
    }
    return result;
}
/**
 * @brief Function that loads Page indicated by pageIndex. Now the cursor starts
 * reading from the new page.
 *
 * @param pageIndex
 */
void Cursor::nextPage(int pageIndex) {
    logger.log("Cursor::nextPage");
    this->page = bufferManager.getPage(this->tableName, pageIndex);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}

Cursor::Cursor(string matrixName, int pageIndex, bool isMatrix) {
    logger.log("Cursor::Cursor (matrix mode)");
    this->tableName = matrixName;
    this->pageIndex = pageIndex;
    this->matrixMode = isMatrix;

    this->page = bufferManager.getMatrixPage(matrixName, pageIndex);
    // For matrix: we typically have rowCount=1, so we can do "row major" in
    // row[0].
    this->pagePointer = 0;
}

void Cursor::nextMatrixPage(int pageIndex) {
    if (!matrixMode) {
        logger.log("Cursor::nextMatrixPage called in table mode");
        return;
    }
    logger.log("Cursor::nextMatrixPage (matrix mode)");
    this->page = bufferManager.getMatrixPage(this->tableName, pageIndex);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}

int Cursor::getElementAtMatrixOffset(int offset) {
    if (!matrixMode) {
        logger.log("Cursor::getElementAtMatrixOffset called in table mode");
        return -1;
    }

    vector<int> matrixData = page.getRow(0);

    if (offset < 0 || offset >= static_cast<int>(matrixData.size())) {
        return -1;
    }

    return matrixData[offset];
}

Cursor::Cursor(string hashedTableName, int pageIndex, int total){
    logger.log("Cursor::Cursor(tempPagefromHashing)");
    this->page = bufferManager.getHashPage(hashedTableName, pageIndex);
    this->pagePointer = 0;
    this->tableName = hashedTableName; //HashTablename_bucket
    // cout << "tableName in Cursor: " << tableName << endl;
    this->pageIndex = pageIndex;
    this->totalhashPages = total;
    // cout << "totalhashPages: " << totalhashPages << endl;
}

vector<int> Cursor::getNextHashrow(){
    logger.log("Cursor::geNext(Hash)");
    // cout << "getNextHashrow called. PageIndex: " << this->pageIndex << " PagePointer: " << this-> pagePointer << endl;
    vector<int> result = this->page.getRow(this->pagePointer);
    // if(result.empty()){
    //     cout << "Empty result" << endl;
    // }
    this->pagePointer++;
    if (result.empty()) {
        if(this->pageIndex < this->totalhashPages - 1){
            this->nextHashPage(this->pageIndex + 1);
            // cout << "Rows over, new pageIndex: " << this->pageIndex + 1 << endl;
            if (!this->pagePointer) {
                result = this->page.getRow(this->pagePointer);
                this->pagePointer++;
            }
        }
    }
    return result;
}

void Cursor::nextHashPage(int pageIndex) {
    logger.log("Cursor::next(Hash)Page");
    this->page = bufferManager.getHashPage(this->tableName, pageIndex);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}
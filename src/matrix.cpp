#include "matrix.h"
#include "global.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

Matrix::Matrix() {
    logger.log("Matrix::Matrix");
    matrixName = "";
    sourceFileName = "";
    dimension = 0;
    totalElements = 0;
    pageCount = 0;
    this->permanent = false;
}

bool Matrix::isMatrixPermanent() const { return this->permanent; }

void Matrix::setMatrixPermanent(bool val) { this->permanent = val; }

void Matrix::makePermanent() {
    logger.log("Matrix::makePermanent");

    string csvPath = "../data/" + this->matrixName + ".csv";
    bool fileAlreadyExisted = false;
    {
        ifstream test(csvPath);
        if (test.is_open()) {
            fileAlreadyExisted = true;
            test.close();
        }
    }

    ofstream fout(csvPath, ios::out | ios::trunc);
    if (!fout.is_open()) {
        cerr << "Error: Cannot open file " << csvPath << " for writing."
             << endl;
        return;
    }

    NO_OF_BLOCK_READ++;
    
    Cursor cursor(this->matrixName, 0, 1);
    vector<int> block;
    vector<int> currentRow;
    long long elementScanned = 0;
    int printedRows = 0;

    while (printedRows < this->dimension) {
        block = cursor.getNextMatrix(); // Fetch next 250 elements
        for (int i : block) {
            currentRow.push_back(i);
            if (currentRow.size() == this->dimension) {
                this->writeRow(currentRow, fout);
                printedRows++;
                currentRow.clear();
            }
            elementScanned++;
        }
    }
    fout.close();

    this->setMatrixPermanent(true);

    cout << "Matrix '" << this->matrixName << "' "
         << (fileAlreadyExisted ? "updated" : "created") << " at " << csvPath
         << endl;
}

Matrix::Matrix(string matrixName) {
    logger.log("Matrix::Matrix");
    this->matrixName = matrixName;
    this->sourceFileName = "../data/" + matrixName + ".csv";
    dimension = 0;
    totalElements = 0;
    pageCount = 0;
}

bool Matrix::updateMatrixInfo(string firstLine) {
    logger.log("Matrix::updateMatrixInfo");

    stringstream s(firstLine);
    string token;
    int columnCount = 0;

    while (getline(s, token, ',')) {
        token.erase(remove_if(token.begin(), token.end(), ::isspace),
                    token.end());
        if (token.empty()) {
            logger.log("Empty token detected in first line.");
            return false;
        }
        columnCount++;
    }

    if (columnCount == 0) {
        cout << "Error: Column count is zero!" << endl;
        return false;
    }

    const int MAX_ELEMENTS_PER_BLOCK =
        (uint)((BLOCK_SIZE * 1000) / (sizeof(int)));

    this->dimension = columnCount;
    this->totalElements = (long long)dimension * dimension;
    this->pageCount =
        (totalElements + MAX_ELEMENTS_PER_BLOCK - 1) / MAX_ELEMENTS_PER_BLOCK;
    this->maxElementsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int)));

    return true;
}

bool Matrix::load() {
    logger.log("Matrix::load");
    // Calculate the maximum number of integers per block based on BLOCK_SIZE.
    const int MAX_ELEMENTS_PER_BLOCK =
        (uint)((BLOCK_SIZE * 1000) / (sizeof(int)));

    // Open the source file to read the first line (header) and update matrix
    // info.
    ifstream fin1(this->sourceFileName, ios::in);
    if (!fin1.is_open()) {
        cout << "Error: Cannot open file " << this->sourceFileName << endl;
        return false;
    }
    string firstLine;
    if (!getline(fin1, firstLine)) {
        logger.log("Failed to read first line from CSV.");
        fin1.close();
        return false;
    }
    if (!this->updateMatrixInfo(firstLine)) {
        logger.log("Failed to update matrix info.");
        fin1.close();
        return false;
    }
    fin1.close();

    // Re-open the file for full processing.
    ifstream fin(this->sourceFileName, ios::in);
    if (!fin.is_open()) {
        cout << "Error: Cannot open file " << this->sourceFileName << endl;
        return false;
    }

    vector<int> pageBuffer;
    int lineCount = 0;
    string line;
    int pageIndex = 0;

    // Process each line in the CSV.
    while (getline(fin, line)) {
        if (line.empty())
            continue;

        stringstream ss(line);
        string token;
        int colCount = 0;

        while (getline(ss, token, ',')) {
            // Remove any whitespace from the token.
            token.erase(remove_if(token.begin(), token.end(), ::isspace),
                        token.end());
            if (token.empty())
                continue;

            int value = stoi(token);
            pageBuffer.push_back(value);
            colCount++;

            // If we've reached the maximum number of elements for this block,
            // write the page to disk using the BufferManager.
            if (pageBuffer.size() == MAX_ELEMENTS_PER_BLOCK) {
                bufferManager.writeMatrixPage(matrixName, pageIndex,
                                              pageBuffer);
                pageIndex++;
                pageBuffer.clear();
            }
        }

        // Ensure that the number of columns in this row matches the expected
        // dimension.
        assert(dimension != 0);
        if (colCount != dimension) {
            cout << "Error: Inconsistent number of columns in matrix file."
                 << endl;
            fin.close();
            return false;
        }
        lineCount++;
    }
    fin.close();

    // Write any remaining data in the buffer as the final page.
    if (!pageBuffer.empty()) {
        bufferManager.writeMatrixPage(matrixName, pageIndex, pageBuffer);
    }

    // Check that the matrix is square.
    if (lineCount != dimension) {
        cout << "Error: Matrix is not square. Rows: " << lineCount
             << ", expected: " << dimension << endl;
        return false;
    }

    this->totalElements = (long long)dimension * dimension;
    return true;
}



/**
 * @brief Print the entire matrix to stdout using row-major order.
 * Uses the Cursor to read each element by (pageIndex, offset).
 */
void Matrix::print() {
    logger.log("Matrix::print");
    int count = min((int)PRINT_COUNT, this->dimension);
    NO_OF_BLOCK_READ++;
    
    Cursor cursor(this->matrixName, 0, 1);
    vector<int> block;
    vector<int> currentRow;
    long long elementScanned = 0;
    int printedRows = 0;

    while (printedRows < count) {
        block = cursor.getNextMatrix(); // Fetch next 250 elements
        for (int i : block) {
            if (((elementScanned) % this->dimension) < count) {
                currentRow.push_back(i);
            }
            if (currentRow.size() == count) {
                this->writeRow(currentRow, cout);
                printedRows++;
                currentRow.clear();
            }
            elementScanned++;
        }
    }
}

/**
 * @brief Get a single element (i, j) by row-major linear offset.
 * Uses a Cursor in matrix mode for reading.
 */
int Matrix::getElement(int i, int j) {
    logger.log("Matrix::getElement");

    const int MAX_ELEMENTS_PER_BLOCK =
        (uint)((BLOCK_SIZE * 1000) / (sizeof(int)));

    int linearIndex = i * dimension + j;
    int pageIndex = linearIndex / MAX_ELEMENTS_PER_BLOCK;
    int offset = linearIndex % MAX_ELEMENTS_PER_BLOCK;

    NO_OF_BLOCK_READ++;
    Cursor c(this->matrixName, pageIndex, true);
    int val = c.getElementAtMatrixOffset(offset);

    return val; // -1 if offset is invalid (as per your Cursor code)
}

int Matrix::getDimension() const { return dimension; }

string Matrix::getMatrixName() const { return matrixName; }

/**
 * @brief Swap elements at (i1, j1) and (i2, j2) by:
 *        1) Reading both pages fully into memory.
 *        2) Exchanging the elements.
 *        3) Writing them back with writeMatrixPage(...).
 */
void Matrix::swapElements(int i1, int j1, int i2, int j2) {
    logger.log("Matrix::swapElements");

    // Compute linear offsets
    int index1 = i1 * dimension + j1;
    int index2 = i2 * dimension + j2;

    const int MAX_ELEMENTS_PER_BLOCK =
        (uint)((BLOCK_SIZE * 1000) / (sizeof(int)));

    // Compute which page + offset for each
    int pageIndex1 = index1 / MAX_ELEMENTS_PER_BLOCK;
    int offset1 = index1 % MAX_ELEMENTS_PER_BLOCK;
    int pageIndex2 = index2 / MAX_ELEMENTS_PER_BLOCK;
    int offset2 = index2 % MAX_ELEMENTS_PER_BLOCK;

    // If both elements are on the same page, we only need to load/write once.
    if (pageIndex1 == pageIndex2) {
        NO_OF_BLOCK_READ++;
        Cursor c(this->matrixName, pageIndex1, true);
        // Entire row for a matrix page is stored in row 0:
        vector<int> &data = bufferManager.loadTempBlock(c.page.getRow(0), 0);

        // Swap

        // for(int i = 0; i < data.size(); i++){
        //     cout << data[i] << " ";
        // }
        int temp = data[offset1];
        data[offset1] = data[offset2];
        data[offset2] = temp;

        // cout <<"Swapping "<<data[offset1]<<" and "<<data[offset2]<<endl;
        // for(int i = 0; i < data.size(); i++){
        //     cout << data[i] << " ";
        // }
        // cout << endl;

        // Write back
        bufferManager.writeMatrixPage(this->matrixName, pageIndex1, data);
    } else {
        // Page 1
        {
            NO_OF_BLOCK_READ++;
            Cursor c1(this->matrixName, pageIndex1, true);
            vector<int> &data1 =
                bufferManager.loadTempBlock(c1.page.getRow(0), 0);

            // Page 2
            NO_OF_BLOCK_READ++;
            Cursor c2(this->matrixName, pageIndex2, true);
            vector<int> &data2 =
                bufferManager.loadTempBlock(c2.page.getRow(0), 1);

            // Swap
            int temp = data1[offset1];
            data1[offset1] = data2[offset2];
            data2[offset2] = temp;

            // Write both pages back
            bufferManager.writeMatrixPage(this->matrixName, pageIndex1, data1);
            bufferManager.writeMatrixPage(this->matrixName, pageIndex2, data2);
        }
    }
}

void Matrix::reverseRows() {
    logger.log("Matrix::reverseRows");

    int lastBlockA = -1;
    int lastBlockB = -1;
    vector<int> dataA, dataB;
    bool isDirtyA = false, isDirtyB = false;

    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension / 2; j++) {
            int index1 = i * dimension + j;
            int pageIndex1 = index1 / maxElementsPerBlock;
            int offset1 = index1 % maxElementsPerBlock;
            int index2 = i * dimension + (dimension - j - 1);
            int pageIndex2 = index2 / maxElementsPerBlock;
            int offset2 = index2 % maxElementsPerBlock;

            if (lastBlockA != pageIndex1) {
                if (isDirtyA) {
                    bufferManager.writeMatrixPage(this->matrixName, lastBlockA, dataA);
                    isDirtyA = false;
                }
                if (lastBlockB == pageIndex1) {
                    bufferManager.writeMatrixPage(this->matrixName, lastBlockB, dataB);
                    isDirtyB = false;
                    lastBlockB = -1;
                }
                Cursor cursorA(this->matrixName, pageIndex1, true);
                NO_OF_BLOCK_READ++;
                dataA = cursorA.getNextMatrix();
                lastBlockA = pageIndex1;
            }
            if (pageIndex1 == pageIndex2) {
                swap(dataA[offset1], dataA[offset2]);
                isDirtyA = true;
                continue;
            }
            if (lastBlockB != pageIndex2) {
                if (isDirtyB) {
                    bufferManager.writeMatrixPage(this->matrixName, lastBlockB, dataB);
                    isDirtyB = false;
                }
                Cursor cursorB(this->matrixName, pageIndex2, true);
                NO_OF_BLOCK_READ++;
                dataB = cursorB.getNextMatrix();
                lastBlockB = pageIndex2;
            }

            swap(dataA[offset1], dataB[offset2]);
            isDirtyA = true;
            isDirtyB = true;
        }
    }

    if (isDirtyA) {
        bufferManager.writeMatrixPage(this->matrixName, lastBlockA, dataA);
    }
    if (isDirtyB) {
        bufferManager.writeMatrixPage(this->matrixName, lastBlockB, dataB);
    }
}


void Matrix::rotate() {
    logger.log("Matrix::rotate");

    transposeInPlaceOptimized();

    reverseRows();
    // for (int i = 0; i < dimension; i++) {
    //     for (int j = 0; j < dimension / 2; j++) {
    //         swapElements(i, j, i, dimension - 1 - j);
    //     }
    // }
}

void Matrix::transposeInPlace() {
    logger.log("Matrix::transposeInPlace");
    for (int i = 0; i < dimension; i++) {
        for (int j = i + 1; j < dimension; j++) {
            swapElements(i, j, j, i);
        }
    }
}

/**
 * @brief Optimized in-place matrix transposition to minimize block accesses.
 * 
 * This function transposes a matrix in place while optimizing the number of block accesses.
 * The optimization is achieved by ensuring that each block is accessed only when necessary.
 * CursorB is not initialized outside any loop to avoid unnecessary block accesses.
 */
void Matrix::transposeInPlaceOptimized() {
    logger.log("Matrix::transposeInPlaceOptimized");

    int lastBlockA = -1;
    int lastBlockB = -1;
    vector<int> dataA, dataB;
    bool isDirtyA = false, isDirtyB = false;

    for (int i = 0; i < dimension; i++) {
        for (int j = i + 1; j < dimension; j++) {
            int index1 = i * dimension + j;
            int pageIndex1 = index1 / maxElementsPerBlock;
            int offset1 = index1 % maxElementsPerBlock;

            int index2 = j * dimension + i;
            int pageIndex2 = index2 / maxElementsPerBlock;
            int offset2 = index2 % maxElementsPerBlock;

            if (lastBlockA != pageIndex1) {
                if (isDirtyA) {
                    bufferManager.writeMatrixPage(this->matrixName, lastBlockA, dataA);
                    isDirtyA = false;
                }
                if (lastBlockB == pageIndex1) {
                    bufferManager.writeMatrixPage(this->matrixName, lastBlockB, dataB);
                    isDirtyB = false;
                    lastBlockB = -1;
                }
                Cursor cursorA(this->matrixName, pageIndex1, true);
                NO_OF_BLOCK_READ++;
                dataA = cursorA.getNextMatrix();
                lastBlockA = pageIndex1;
            }

            if (pageIndex1 == pageIndex2) {
                swap(dataA[offset1], dataA[offset2]);
                isDirtyA = true;
                continue;
            }

            if (lastBlockB != pageIndex2) {
                if (isDirtyB) {
                    bufferManager.writeMatrixPage(this->matrixName, lastBlockB, dataB);
                    isDirtyB = false;
                }
                Cursor cursorB(this->matrixName, pageIndex2, true);
                NO_OF_BLOCK_READ++;
                dataB = cursorB.getNextMatrix();
                lastBlockB = pageIndex2;
            }

            swap(dataA[offset1], dataB[offset2]);
            isDirtyA = true;
            isDirtyB = true;
        }
    }

    if (isDirtyA) bufferManager.writeMatrixPage(this->matrixName, lastBlockA, dataA);
    if (isDirtyB) bufferManager.writeMatrixPage(this->matrixName, lastBlockB, dataB);
}


void Matrix::unload() {
    logger.log("Matrix::unload");

    for (int pageIndex = 0; pageIndex < this->pageCount; pageIndex++) {
        bufferManager.deleteFile(this->matrixName, pageIndex);
    }
}

/**
 * @brief This function returns one row of the Matrix using the cursor object.
 * It returns an empty row is all rows have been read.
 *
 * @param cursor
 * @return vector<int>
 */
void Matrix::getNextPage(Cursor *cursor) {
    logger.log("Matrix::getNext (Matrix)");

    if (cursor->pageIndex < this->pageCount - 1) {
        cursor->nextMatrixPage(cursor->pageIndex + 1);
    }
    NO_OF_BLOCK_READ++;
}
#include "global.h"
#include "matrix.h"
#include <algorithm>
#include <iostream>
using namespace std;

bool syntacticParseCROSSTRANSPOSE() {
    logger.log("syntacticParseCROSSTRANSPOSE");
    if (tokenizedQuery.size() != 3) {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = CROSSTRANSPOSE;
    parsedQuery.crossTransposeMatrixName1 = tokenizedQuery[1];
    parsedQuery.crossTransposeMatrixName2 = tokenizedQuery[2];
    return true;
}

bool semanticParseCROSSTRANSPOSE() {
    logger.log("semanticParseCROSSTRANSPOSE");
    if (!matrixCatalogue.isMatrix(parsedQuery.crossTransposeMatrixName1)) {
        cout << "SEMANTIC ERROR: Matrix1 does not exist." << endl;
        return false;
    }
    if (!matrixCatalogue.isMatrix(parsedQuery.crossTransposeMatrixName2)) {
        cout << "SEMANTIC ERROR: Matrix2 does not exist." << endl;
        return false;
    }
    return true;
}

void executeCROSSTRANSPOSE() {
    logger.log("executeCROSSTRANSPOSE");

    Matrix *A = matrixCatalogue.getMatrix(parsedQuery.crossTransposeMatrixName1);
    Matrix *B = matrixCatalogue.getMatrix(parsedQuery.crossTransposeMatrixName2);

    int oldDimA = A->getDimension();
    int oldDimB = B->getDimension();
    int oldMaxElementsPerBlockA = A->maxElementsPerBlock;
    int oldPageCountA = A->pageCount;
    int oldPageCountB = B->pageCount;
    int oldMaxElementsPerBlockB = B->maxElementsPerBlock;

    A->transposeInPlaceOptimized();
    B->transposeInPlaceOptimized();

    int commonPages = std::min(oldPageCountA, oldPageCountB);
    for (int pageIndex = 0; pageIndex < commonPages; pageIndex++) {
        NO_OF_BLOCK_READ++;
        NO_OF_BLOCK_READ++;
        Cursor cursorA(A->getMatrixName(), pageIndex, true);
        Cursor cursorB(B->getMatrixName(), pageIndex, true);


        vector<int>& pageDataA = bufferManager.loadTempBlock(cursorA.page.getRow(0), 0);
        vector<int>& pageDataB = bufferManager.loadTempBlock(cursorB.page.getRow(0), 1);

        bufferManager.writeMatrixPage(A->getMatrixName(), pageIndex, pageDataB);
        bufferManager.writeMatrixPage(B->getMatrixName(), pageIndex, pageDataA);

    }

    if (oldPageCountA < oldPageCountB) {
        for (int pageIndex = oldPageCountA; pageIndex < oldPageCountB; pageIndex++) {
            NO_OF_BLOCK_READ++;
            Cursor cursorB(B->getMatrixName(), pageIndex, true);
            vector<int>& pageData = bufferManager.loadTempBlock(cursorB.page.getRow(0), 0); //here

            bufferManager.writeMatrixPage(A->getMatrixName(), pageIndex, pageData);

            bufferManager.deleteFile(B->getMatrixName(), pageIndex);
        }
    } else if (oldPageCountA > oldPageCountB) {
   
        for (int pageIndex = oldPageCountB; pageIndex < oldPageCountA; pageIndex++) {
            NO_OF_BLOCK_READ++;
            Cursor cursorA(A->getMatrixName(), pageIndex, true);
            vector<int>& pageData = bufferManager.loadTempBlock(cursorA.page.getRow(0), 0); //here
            bufferManager.writeMatrixPage(B->getMatrixName(), pageIndex, pageData);
            bufferManager.deleteFile(A->getMatrixName(), pageIndex);
        }
    }

    const int MAX_ELEMENTS_PER_BLOCK =
        (uint)((BLOCK_SIZE * 1000) / (sizeof(int)));


    A->dimension = oldDimB;
    A->totalElements = (long long)oldDimB * oldDimB;
    A->pageCount = (A->totalElements + MAX_ELEMENTS_PER_BLOCK - 1) / MAX_ELEMENTS_PER_BLOCK;
    A->maxElementsPerBlock = oldMaxElementsPerBlockB;

    B->dimension = oldDimA;
    B->totalElements = (long long)oldDimA * oldDimA;
    B->pageCount = (B->totalElements + MAX_ELEMENTS_PER_BLOCK - 1) / MAX_ELEMENTS_PER_BLOCK;
    B->maxElementsPerBlock = oldMaxElementsPerBlockA;

    cout << "CROSSTRANSPOSE completed: " << A->getMatrixName()
         << " now holds " << B->getMatrixName()
         << "'s transpose and vice versa." << endl;
}

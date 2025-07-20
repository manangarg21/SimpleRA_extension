#include "cursor.h"
#include "global.h"
#include "matrix.h"
#include <iostream>
#include <sys/stat.h>
using namespace std;

/**
 * @brief SYNTAX: CHECKANTISYM <matrix name1> <matrix name2>
 */
bool syntacticParseCHECKANTISYM() {
    logger.log("syntacticParseCHECKANTISYM");
    if (tokenizedQuery.size() != 3) {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = CHECKANTISYM;
    parsedQuery.checkAntisymMatrixName1 = tokenizedQuery[1];
    parsedQuery.checkAntisymMatrixName2 = tokenizedQuery[2];
    return true;
}

/**
 * @brief Semantic parsing for CHECKANTISYM.
 * Checks that both matrix files exist and that they have the same dimensions.
 */
bool semanticParseCHECKANTISYM() {
    logger.log("semanticParseCHECKANTISYM");
    if (!matrixCatalogue.isMatrix(parsedQuery.checkAntisymMatrixName1)) {
        cout << "SEMANTIC ERROR: Matrix1 does not exist." << endl;
        return false;
    }
    if (!matrixCatalogue.isMatrix(parsedQuery.checkAntisymMatrixName2)) {
        cout << "SEMANTIC ERROR: Matrix2 does not exist." << endl;
        return false;
    }

    return true;
}

/**
 * @brief Executes the CHECKANTISYM command.
 * Checks whether matrix A equals -1 * B^T (i.e., A(i,j) == -B(j,i) for every
 * i,j). Prints "True" if the condition is satisfied, "False" otherwise.
 */
// void executeCHECKANTISYM() {
//     logger.log("executeCHECKANTISYM_blockBased");

//     Matrix *A = matrixCatalogue.getMatrix(parsedQuery.checkAntisymMatrixName1);
//     Matrix *B = matrixCatalogue.getMatrix(parsedQuery.checkAntisymMatrixName2);

//     int n = A->getDimension();
//     if (n != B->getDimension()) {
//         cout << "SEMANTIC ERROR: Matrices dimensions do not match" << endl;
//         return;
//     }
//     if (n > 250) {
//         const int BLOCK_SIZE = 250;
//         long long totalElements = (long long)n * n;
//         long long totalPages = (totalElements + BLOCK_SIZE - 1) / BLOCK_SIZE;

//         bool antisym = true;

//         for (long long pageIndexA = 0; pageIndexA < totalPages && antisym;
//              pageIndexA++) {
//             Cursor cursorA(A->getMatrixName(), pageIndexA, true);
//             vector<int> blockA = cursorA.page.getRow(0);

//             long long startA = pageIndexA * BLOCK_SIZE;
//             long long endA = std::min(startA + BLOCK_SIZE, totalElements);

//             for (long long pageIndexB = 0; pageIndexB < totalPages && antisym;
//                  pageIndexB++) {
//                 Cursor cursorB(B->getMatrixName(), pageIndexB, true);
//                 vector<int> blockB = cursorB.page.getRow(0);

//                 long long startB = pageIndexB * BLOCK_SIZE;
//                 long long endB = std::min(startB + BLOCK_SIZE, totalElements);

//                 // Now we compare all pairs of offsets (offsetA in [startA,
//                 // endA)) with offsetB in [startB, endB)) that correspond to (i,
//                 // j) <-> (j, i). We'll do a single pass over blockA, computing
//                 // the transposed offset in B.

//                 for (long long localOffsetA = 0;
//                      localOffsetA < (endA - startA) && antisym;
//                      localOffsetA++) {

//                     long long globalOffsetA = startA + localOffsetA;
//                     long long i = globalOffsetA / n;
//                     long long j = globalOffsetA % n;
//                     long long globalOffsetB = (long long)j * n + i;

//                     if (globalOffsetB >= startB && globalOffsetB < endB) {
//                         long long localOffsetB = globalOffsetB - startB;
//                         if (blockA[localOffsetA] != -blockB[localOffsetB]) {
//                             antisym = false;
//                         }
//                     }
//                 }
//             }
//         }

//         cout << (antisym ? "True" : "False") << endl;
//     } 
    
//     else {
//         Cursor aCursor(A->getMatrixName(), 0, true);
//         Cursor bCursor(B->getMatrixName(), 0, true);

//         bool antisym = true;
//         // Loop over every logical coordinate (i,j)
//         for (int i = 0; i < n && antisym; i++) {
//             for (int j = 0; j < n && antisym; j++) {
//                 int indexA = i * n + j;
//                 int pageIndexA = indexA / 250;
//                 int offsetA = indexA % 250;
//                 aCursor.nextMatrixPage(pageIndexA);
//                 int a_val = aCursor.getElementAtMatrixOffset(offsetA);

//                 // For B, use transposed index (B(j,i))
//                 int indexB = j * n + i;
//                 int pageIndexB = indexB / 250;
//                 int offsetB = indexB % 250;
//                 bCursor.nextMatrixPage(pageIndexB);
//                 int b_val = bCursor.getElementAtMatrixOffset(offsetB);

//                 if (a_val != -b_val)
//                     antisym = false;
//             }
//         }
//         cout << (antisym ? "True" : "False") << endl;
//     }
// }

void executeCHECKANTISYM() {
    logger.log("executeCHECKANTISYM_Optimized");

    Matrix *A = matrixCatalogue.getMatrix(parsedQuery.checkAntisymMatrixName1);
    Matrix *B = matrixCatalogue.getMatrix(parsedQuery.checkAntisymMatrixName2);

    int n = A->getDimension();
    if (n != B->getDimension()) {
        cout << "SEMANTIC ERROR: Matrices dimensions do not match" << endl;
        return;
    }

    const int CURR_BLOCK_SIZE = (uint)((BLOCK_SIZE * 1000) / (sizeof(int)));
    long long totalElements = (long long)n * n;
    long long totalPages = (totalElements + CURR_BLOCK_SIZE - 1) / CURR_BLOCK_SIZE;

    bool antisym = true;

    int lastBlockB = -1;  // Track last accessed block for B
    vector<int> blockB;   // Store B's currently loaded block

    for (long long pageIndexA = 0; pageIndexA < totalPages && antisym; pageIndexA++)
    {
        NO_OF_BLOCK_READ++;
        Cursor cursorA(A->getMatrixName(), pageIndexA, true);
        vector<int> blockA = cursorA.page.getRow(0);

        long long startA = pageIndexA * CURR_BLOCK_SIZE;
        long long endA   = min(startA + CURR_BLOCK_SIZE, totalElements);


        for (long long localOffsetA = 0; 
             localOffsetA < (endA - startA) && antisym; 
             localOffsetA++)
        {
            long long globalOffsetA = startA + localOffsetA;

            long long i = globalOffsetA / n;
            long long j = globalOffsetA % n;

            long long globalOffsetB = (long long)j * n + i;
            int pageIndexB = globalOffsetB / CURR_BLOCK_SIZE;
            int offsetB = globalOffsetB % CURR_BLOCK_SIZE;

            // Load B's block only if necessary
            if (lastBlockB != pageIndexB) {
                NO_OF_BLOCK_READ++;
                Cursor cursorB(B->getMatrixName(), pageIndexB, true);
                blockB = cursorB.getNextMatrix();
                lastBlockB = pageIndexB;
            }


            // Check antisymmetry condition
            if (blockA[localOffsetA] != -blockB[offsetB]) {
                antisym = false;
                break;
            }
        }
    }

    cout << (antisym ? "True" : "False") << endl;
}

#include "global.h"
#include "matrix.h"

/**
 * @brief SYNTAX: PRINT MATRIX <matrix name>
 */

bool syntacticParsePRINTMATRIX() {
    logger.log("syntacticParsePRINTMATRIX");
    if (tokenizedQuery.size() != 3) {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if (tokenizedQuery[1] != "MATRIX") {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = PRINTMATRIX;
    parsedQuery.printMatrixName = tokenizedQuery[2];
    return true;
}

bool semanticParsePRINTMATRIX() {
    logger.log("semanticParsePRINTMATRIX");
    if (!matrixCatalogue.isMatrix(parsedQuery.printMatrixName)) {
        cout << "SEMANTIC ERROR: Matrix does not exist." << endl;
        return false;
    }
    return true;
}

void executePRINTMATRIX() {
    logger.log("executePRINTMATRIX");
    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.printMatrixName);
    matrix->print();
}

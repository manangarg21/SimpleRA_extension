#include "global.h"
#include "matrix.h"

/**
 * @brief SYNTAX: LOAD MATRIX <matrix name>
 */
bool syntacticParseLOADMATRIX() {
    logger.log("syntacticParseLOADMATRIX");
    if (tokenizedQuery.size() != 3) {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if (tokenizedQuery[1] != "MATRIX") {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = LOADMATRIX;
    parsedQuery.loadMatrixName = tokenizedQuery[2];
    return true;
}

bool semanticParseLOADMATRIX() {
    logger.log("semanticParseLOADMATRIX");
    if (matrixCatalogue.isMatrix(parsedQuery.loadMatrixName)) {
        cout << "SEMANTIC ERROR: Matrix already exists" << endl;
        return false;
    }
    if (tableCatalogue.isTable(parsedQuery.loadMatrixName)) {
        cout << "SEMANTIC ERROR: Matrix name clashes with existing relation name" << endl;
        return false;
    }
    if (!isFileExists(parsedQuery.loadMatrixName)) {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeLOADMATRIX() {
    logger.log("executeLOADMATRIX");
    Matrix *matrix = new Matrix(parsedQuery.loadMatrixName);
    if (matrix->load()) {
        matrixCatalogue.insertMatrix(matrix);
        cout << "Loaded Matrix " << matrix->matrixName
             << ". Dimension: " << matrix->dimension << " x "
             << matrix->dimension << endl;
    } else {
        cout << "Error loading matrix " << parsedQuery.loadMatrixName << endl;
        delete matrix;
    }
    return;
}

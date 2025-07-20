#include "global.h"
#include "matrix.h"
#include <iostream>
using namespace std;

bool syntacticParseROTATE() {
    logger.log("syntacticParseROTATE");
    if (tokenizedQuery.size() != 2) {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = ROTATE;
    parsedQuery.rotateMatrixName = tokenizedQuery[1];
    return true;
}

bool semanticParseROTATE() {
    logger.log("semanticParseROTATE");
    if (!matrixCatalogue.isMatrix(parsedQuery.rotateMatrixName)) {
        cout << "SEMANTIC ERROR: Matrix is not loaded." << endl;
        return false;
    }
    return true;
}

void executeROTATE() {
    logger.log("executeROTATE");
    Matrix *m = matrixCatalogue.getMatrix(parsedQuery.rotateMatrixName);
    m->rotate();
    cout << "Matrix " << m->getMatrixName() << " rotated successfully." << endl;
}

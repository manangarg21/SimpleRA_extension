#include "global.h"
#include "matrix.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
using namespace std;

/**
 * @brief Syntactic parser for EXPORT MATRIX command.
 * Expected syntax: EXPORT MATRIX <matrix name>
 */
bool syntacticParseEXPORTMATRIX() {
    logger.log("syntacticParseEXPORTMATRIX");
    // Expect exactly three tokens: "EXPORT", "MATRIX", and the matrix name.
    if (tokenizedQuery.size() != 3) {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if (tokenizedQuery[1] != "MATRIX") {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = EXPORTMATRIX;
    parsedQuery.exportMatrixName = tokenizedQuery[2];
    return true;
}

/**
 * @brief Semantic parser for EXPORT MATRIX command.
 * Checks that at least one temporary page exists for the matrix.
 */
bool semanticParseEXPORTMATRIX() {
    logger.log("semanticParseEXPORTMATRIX");
    // Construct the filename for temporary Page 0.
    if (!matrixCatalogue.isMatrix(parsedQuery.exportMatrixName)) {
        cout << "SEMANTIC ERROR: Matrix does not exist." << endl;
        return false;
    }
    return true;
}

/**
 * @brief Executes the EXPORT MATRIX command.
 *
 * Reads the current state of the matrix (from its temporary pages) and writes
 * it out in row-major order to a CSV file in the /data directory.
 */
void executeEXPORTMATRIX() {
    logger.log("executeEXPORTMATRIX");

    // Create a Matrix object using the matrix name from the query.
    Matrix *m = matrixCatalogue.getMatrix(parsedQuery.exportMatrixName);
    // IMPORTANT: Do NOT call m.load() if you intend to preserve modifications;
    // however, to update metadata (dimension, etc.) we call load() here.

    int dim = m->getDimension();
    if (dim <= 0) {
        cout << "Error: Invalid matrix dimension." << endl;
        return;
    }

    m->makePermanent();

    
}

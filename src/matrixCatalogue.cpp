#include "global.h"

bool MatrixCatalogue::isMatrix(string matrixName) {
    logger.log("MatrixCatalogue::matrixName");
    if (this->matrices.count(matrixName))
        return true;
    return false;
}

void MatrixCatalogue::insertMatrix(Matrix *matrix) {
    logger.log("MatrixCatalogue::insertMatrix");
    this->matrices[matrix->matrixName] = matrix;
}

Matrix *MatrixCatalogue::getMatrix(string matrixName) {
    logger.log("MatrixCatalogue::getMatrix");
    Matrix *matrix = this->matrices[matrixName];
    return matrix;
}

void MatrixCatalogue::deleteMatrix(string matrixName) {
    logger.log("MatrixCatalogue::deleteMatrix");
    this->matrices[matrixName]->unload();
    delete this->matrices[matrixName];
    this->matrices.erase(matrixName);
}

MatrixCatalogue::~MatrixCatalogue() {
    logger.log("MatrixCatalogue::~MatrixCatalogue");
    for (auto pair : this->matrices) {
        pair.second->unload();
        delete pair.second;
    }
}
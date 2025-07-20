#ifndef MATRIX_H
#define MATRIX_H

#include "cursor.h"

#include <string>
#include <vector>
using namespace std;

class Matrix {
  public:
    string sourceFileName;
    string matrixName;       // Name of the matrix
    int dimension;           // n, for an n x n matrix
    long long totalElements; // Should equal n*n
    int pageCount;           // Number of pages after blockification
    int maxElementsPerBlock; // Maximum number of elements that can be stored in
                             // a block
    bool permanent = false;  // Whether the matrix is permanent or not

    Matrix();
    Matrix(string matrixName);

    bool updateMatrixInfo(string firstLine);
    bool load();
    // bool blockify();
    void print();
    int getElement(int i, int j);
    int getDimension() const;
    string getMatrixName() const;
    void rotate();
    void transposeInPlace();
    void swapElements(int i1, int j1, int i2, int j2);
    void unload();

    bool isMatrixPermanent() const;
    void setMatrixPermanent(bool val);

    void makePermanent();
    void getNextPage(Cursor *cursor);
    
    void transposeInPlaceOptimized(); 
    void reverseRows();

    /**
     * @brief Static function that takes a vector of valued and prints them out
     * in a comma seperated format.
     *
     * @tparam T current usaages include int and string
     * @param row
     */
    template <typename T> void writeRow(vector<T> row, ostream &fout) {
        logger.log("Table::printRow");
        for (int columnCounter = 0; columnCounter < row.size();
             columnCounter++) {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
        }
        fout << endl;
    }
};

#endif

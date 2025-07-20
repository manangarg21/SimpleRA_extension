#include "executor.h"
#pragma once

extern float BLOCK_SIZE;
extern uint BLOCK_COUNT;
extern uint BLOCK_COUNT_P2; //Q: should this be modiified?
extern uint PRINT_COUNT;
extern vector<string> tokenizedQuery;
extern ParsedQuery parsedQuery;
extern MatrixCatalogue matrixCatalogue;
extern TableCatalogue tableCatalogue;
extern BufferManager bufferManager;

// for printing blocks accessed
extern int NO_OF_BLOCK_READ;
extern int NO_OF_BLOCK_WRITTEN;
extern int NO_OF_BLOCK_ACCESSED;
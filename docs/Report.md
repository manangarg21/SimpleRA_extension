# DS25 Project Phase 1 Report Team 9

## Introduction
This phase involves extending the program to support matrices in addition to tables. Unlike tables, matrices do not have column names and consist solely of integer values. This enhancement is aimed to allow the application to handle both data structures effectively.

## Assumptions
- A file is either matrix or table. It cannot be put in both.
- Matrix is a square matrix only.
- The term page and block will be used interchangabely in this Report and it refers to the abstarction of memory unit of data.
- This report is written is respect to block size contraints of 1 x 1000 bytes. For a matrix of ints this is 250 ints. 
- n is considered as dimension of matrix.

## System Design
### How matrices are stored in the system.
- Matrices in the system are stored using an unspanned row-major organization. This means that:
    - Elements are arranged sequentially by row in a linearized format.
    - Each row is stored contiguously in memory, unless it exceeds the designated block size.
    - Rows that exceed a single block are split across multiple blocks, but no two rows share a block unless they fit entirely.
    - Row-wise access is highly efficient, while column-wise access incurs additional memory traversal costs.

### Block/page design considerations.
These were some of our considerations that affected our system design choices.
- Our implementation **minimises storage**, such that every block except the last one is filled.
- It enables **fast access to an operation like load** which had a constraint of reading input line by line. It also lended itself to the constraints of **using data worth of upto two temporary blocks in main memory**. 
- It also has **straightforward indexing qualities**, compared to other formats.
- It has a **simplified design** that can be extended easily in teh future if contraints change.

- These features were chosen at the expense of certain other operations that may have been more efficent had another design choice (eg: columnar, submatrix) been adopted.

### Memory Usage and usage of temporary blocks


### Data Storage Mechanism
- We have create a new class matrix, with methods specific to it. We also have a matrix Catalogue that keeps tracks of matrices added to the system. 

- Each matrix is uniquely identified and stored as a sequence of fixed-size blocks (kept in disk).
    Nomenclature format for matrix page in temp: matrixName_

- Block indices are used for direct lookup and efficient access. There are simple mappings that exist to check for:
    1. The page block in which an element of index i, j belong 2
    2. Elements stored in a page block. 

- Cursors facilitate block traversal. They are the interface to read.

- Supporting metadata is present in Matrix class as variables. These include:
    - `string matrixName`: Name of the matrix.
    - `int dimension`: Dimension `n`, for an `n x n` matrix.
    - `long long totalElements`: Total number of elements, should equal `n * n`.
    - `int pageCount`: Number of pages after blockification.
    - `int maxElementsPerBlock`: Maximum number of elements that can be stored in a block. Can be used to recognise storage of a matrix if this changes later on. 
    - `bool permanent`: Whether the matrix is permanent or not.

### Handling of large matrices.
Here are some of the techniques used.
- Block-Based Computation
    - Matrices exceeding block size constraints (e.g., ) are processed block-by-block.
    - A two-cursor strategy ensures that only the necessary blocks are loaded into memory, reducing RAM consumption.This also meets the question constraints of accessing upto only 2 pages at a time.

- Optimized Transposition and Row Reversal
    -  Matrix transposition operations are optimized to minimize block accesses through change tracking. More on this in the functions.
    - Row reversal operations leverage active block tracking, avoiding redundant memory loads.

- Scalability and Performance Considerations
    - Scalability is achieved through block-aware computations, which prevent excessive memory usage.
    - Out-of-core computation techniques allow handling of large datasets by avoiding full matrix loads into memory.


## Implementation Details and block Accesses

### SOURCE Command
- Pupose: To read and execute commands from a file (.ra format) in folder ../data/. All commands supported normally inclusing QUIT should be supported here too. 
- Excecution flow:
    - Opens the source file (located in ../data/) saved in .ra format.
    - Reads commands line by line, assuming '\n' as the delimiter.
    - Tokenizes the command using regex ([^\\s,]+).
    - Checks for "QUIT", stopping execution if found.
    - Handles syntax validation:
        - If the command has only one token, it logs a syntax error.
        - Otherwise, it proceeds to parse and execute the command.
    - Calls syntacticParse(), semanticParse(), and executeCommand() for valid queries.

### LOAD MATRIX
- Purpose: to take data from a csv file and store it as a matrix for the system. 
- Implemented mainly with the help of two functions:
    - `load()`:
        - Reads the first line of the file to extract matrix metadata. According to the specifications put as taking input line by line.
        - Calls `updateMatrixInfo(firstLine)` to set dimensions.
        - Calls `blockify()` to store the matrix as blocks.
    - `blockify()`:
        - Reads the matrix file line by line.
        - Tokenizes CSV data and converts it into integers.
        - Stores elements in a buffer until the block size (with current constraints 250 elements) is reached.
        - Writes a block to disk using `bufferManager.writeMatrixPage()` when full.
        - Repeats until the entire matrix is stored in blocks.
- Page design optimised for such a command and associated constraints.

#### BLOCK ACCESSES
- Total elements in matrix: n<sup>2</sup>
- Elements per block: 250
- Total block writes: ⌈(n<sup>2</sup> / 250)⌉ (ceil division)

Each block is written once during `blockify()`, so the number of block accesses (writes) equals the total number of blocks.

### PRINT MATRIX
It works as follows:
- Sets variable PRINT_COUNT = 20 as per specifications
- Prints the first PRINT_COUNT × PRINT_COUNT elements of the matrix, or the entire matrix if its dimension <= PRINT_COUNT
- Uses a cursor to sequentially fetch matrix blocks.
- Processes elements row-wise, storing them in vector currentRow until count columns are reached.
- Writes rows to output (cout) and continues fetching blocks until count rows are printed.

#### BLOCK ACCESSES
- n<sup>2</sup>/250


### EXPORT MATRIX
- via function `makePermanent()`:
    - Checks if a CSV file already exists.
    - Opens a new CSV file (overwriting if necessary).
    - Writes elements row by row in comma-separated format.
    - Calls `getElement(i, j)` to fetch each element.
    - Marks the matrix as permanent in metadata.

- Page design very convienent for this kind of command. Each row accessed and directly put to format to export. 

#### BLOCK ACCESSES
- Each block must be accessed = ⌈(n<sup>2</sup> / 250)⌉ (ceil division)

### ROTATE
- Performs a 90-degree rotation of the matrix.
- Implemented in two steps.
- `transposeInPlaceOptimized()`: Swaps elements `A[i][j]` with `A[j][i]` (in-place transposition).
    - Determine block locations for both elements.
    - Load blocks only when needed:
        - If blockA is already loaded, reuse it.
        - If blockB is the same as blockA, perform an in-block swap.
        - Otherwise, load blockB only when switching to a new block.
    - Use dirty bits (isDirtyA, isDirtyB):
        - Mark blocks as modified instead of immediately writing them back.
        - Write blocks only when switching to a different block.
    - Avoid redundant reads/writes:
        - Instead of reading and writing twice per swap (total 4 accesses per swap),
        - Reads once per block, writes once per block, significantly reducing I/O.
- `reverseRows()`: Reverses elements within each row.
    - Determine block locations for both elements.
    - Load blockA only when needed:
        - If the next element belongs to the same block, reuse it.
        - Check if blockB is the same as blockA:
            - If yes, swap in memory.
            - If no, load blockB only when switching to a new block.
    - Use dirty bits (isDirtyA, isDirtyB):
        - Mark blocks as modified but delay writing until necessary.
        - Write only when switching to a different block, reducing disk I/O.
    - Minimize redundant reads/writes:
        - Instead of reading and writing twice per swap,
        - Each block is read once and written back only once when necessary.

- Page design not fully optimised for this method (specifically the transpose part), but modifications have it faster that brute force. Also each methods has pros and cons with this sort of command. 

#### BLOCK ACCESSES

- O(2*n) for smaller matrices
- O(n<sup>2</sup>) for big matrices


### CROSS TRANSPOSE
- Retrieve both matrices `A` and `B` from the catalogue.
- Fetch their metadata, including dimensions and block details.
- Transpose both matrices in-place using `transposeInPlaceOptimized()`.
    - This function logic was elaborated in the explanation for the previous command. 
- Swap corresponding blocks of `A` and `B`.
    - Iterates over the common number of blocks.
    - Uses two cursors to fetch pages of `A` and `B` efficiently.
    - Swaps their contents and writes them back to disk.
- If one matrix has more pages than the other, copy over remaining pages.
    - Ensures no data loss by moving the extra blocks.
    - Deletes unnecessary blocks from the larger matrix.
- Update metadata to reflect new dimensions and structure.
    - Swaps dimension, total elements, page count, and max elements per block.
    - Ensures that `A` now holds `B^T` and `B` holds `A^T`.

#### BLOCK ACCESSES

- O(4*n) for smaller matrices
- O(2*n<sup>2</sup>) for bigger matrices


### CHECK ANTISYMMETRY
Purpose: For 2 matrixes `A` and `B` check if `A = -1 × B^T`
- Retrieve matrices `A` and `B` from the catalogue.
- Ensure both matrices exist and have matching dimensions.
- Compute total pages needed based on block size.
- Determine how many blocks are required to store the matrix elements.
- Iterate over pages of `A` and fetch corresponding elements from `B^T`.
    - Use one cursor for `A` (moving sequentially).
    - Track the last accessed block of `B` to avoid redundant reads.
- Compare elements at `(i, j)` in `A` with `-1 × B(j, i)`.
    - Compute the transposed index of `B` dynamically.
    - Fetch the block of `B` only when necessary (avoiding redundant loads).
- Terminate early if an antisymmetry violation is found.
    - If any element `A[i][j] ≠ -B[j][i]`, the check stops immediately.

#### BLOCK ACCESSES

- O(n<sup>2</sup>)


### Error Handling and Logging
- Overall error handling done. This is especially at the time of loading a matrix- when we ensure a valid matrix enters a system.
- Also note: the same csv file cannot be supported as both a matrix and a table in our implementation. There is error handling across functions accordingly.

#### SOURCE
Handles errors related to file operations and syntax:
- File does not exist or cannot be opened → Outputs "ERROR: Unable to open file" and stops execution.
- Incorrect command formatting:
    - If the command contains only one token (except "QUIT"), it prints "SYNTAX ERROR".
    - If the command is empty, it is skipped.

#### LOAD MATRIX
Handles errors related to file parsing and matrix structure:
- File not found or cannot be opened → Logs "Error: Cannot open file" and exits.
- Incorrectly formatted first line → Logs "Failed to read first line from CSV." and stops.
- Inconsistent number of columns in rows →
    - If a row has a different column count than expected, it prints "Error: Inconsistent number of columns in matrix file." and stops.
- Matrix is not square →
    - If the row count does not match the expected dimension, prints "Error: Matrix is not square." and stops.

#### PRINT MATRIX
Ensures valid access and prevents infinite loops:
- If cursor reaches the end of available blocks before printing 20 rows × 20 columns, it terminates gracefully.
- Handles cases where fewer than 20 rows exist, ensuring only available rows are printed.

#### EXPORT MATRIX
Handles file access and write errors.
- File cannot be opened for writing → Prints "Error: Cannot open file [filename] for writing." and stops execution.
- Metadata issues (invalid dimension, missing matrix reference) →
    - If matrix dimension is ≤ 0, prints "Error: Invalid matrix dimension." and stops.

#### ROTATE
Ensures smooth execution and prevents unnecessary block writes:
- If matrix metadata is incorrect (e.g., negative dimension, missing data), it logs an error before starting the operation.
- Dirty bits minimize unnecessary writes, preventing corruption in case of premature failures.

#### CROSS TRANSPOSE
Handles dimension mismatches and missing matrices:
- Matrix dimensions do not match → Stops execution to prevent incorrect results.
- If a page swap fails, logs an error and ensures no half-written data is left.
- Ensures all metadata updates happen correctly to prevent desynchronization between file storage and matrix dimensions.

#### executeCHECKANTISYM()
Ensures logical consistency when checking antisymmetry:
- Matrix dimensions mismatch → Prints "SEMANTIC ERROR: Matrices dimensions do not match" and stops execution.
- Ensures that B[j][i] is fetched correctly and not accessed out of bounds.
- Prevents unnecessary reads by tracking last accessed block (lastBlockB), avoiding redundant memory overhead.


## Contributions
All members collaborated closely, discussing and exploring various design ideas. While contributions were fluid, the general responsibilities were as follows:
- Irfan Ukani (2023201039): Setting up of Matrix Catalouge and class. Worked extensively on Cursor functionality. Worked on several functions such as PRINT, EXPORT, etc. Also worked on integration of cursor and page management with base codebase.
- Manan Garg (2022101109): Worked on creation base codebase. Worked on several functions such as EXPORT, ANTISYMMETRY, ROTATE, etc. Worked on error handling. Also involved in setting up Matrix class.
- Namrata Baliga (2022101021): Worked on Buffer Manager functionality and some cursor functionality integration. Worked on several functions such as ROTATE, CROSS TRANSPOSE, ANTISYMMETRY, etc, and optimising them.

## Testing
This is the results of some of our testing so far.
> LOAD MATRIX NEW

Loaded Matrix NEW. Dimension: 3 x 3

No. of Block Read = 0

No. of Block Written = 1

No. of Block Accessed = 1

> LOAD MATRIX NEWANTI

Loaded Matrix NEWANTI. Dimension: 3 x 3

No. of Block Read = 0

No. of Block Written = 1

No. of Block Accessed = 1

> LOAD MATRIX OG

Loaded Matrix OG. Dimension: 251 x 251

No. of Block Read = 0

No. of Block Written = 253

No. of Block Accessed = 253

> LOAD MATRIX ANTI

Loaded Matrix ANTI. Dimension: 251 x 251

No. of Block Read = 0

No. of Block Written = 253

No. of Block Accessed = 253

> LOAD MATRIX VERYBIG

Loaded Matrix VERYBIG. Dimension: 1000 x 1000

No. of Block Read = 0

No. of Block Written = 4000

No. of Block Accessed = 4000

> LOAD MATRIX VERYBIGANTI

Loaded Matrix VERYBIGANTI. Dimension: 1000 x 1000

No. of Block Read = 0

No. of Block Written = 4000

No. of Block Accessed = 4000

> CROSSTRANSPOSE VERYBIG VERYBIGANTI

CROSSTRANSPOSE completed: VERYBIG now holds VERYBIGANTI's transpose and vice versa.

No. of Block Read = 1011990

No. of Block Written = 1011990

No. of Block Accessed = 2023980

> CHECKANTISYM VERYBIG VERYBIGANTI

True

No. of Block Read = 1004000

No. of Block Written = 0

No. of Block Accessed = 1004000

> PRINT MATRIX VERYBIG

-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19, -20
-1001, -1002, -1003, -1004, -1005, -1006, -1007, -1008, -1009, -1010, -1011, -1012, -1013, -1014, -1015, -1016, -1017, -1018, -1019, -1020
-2001, -2002, -2003, -2004, -2005, -2006, -2007, -2008, -2009, -2010, -2011, -2012, -2013, -2014, -2015, -2016, -2017, -2018, -2019, -2020
-3001, -3002, -3003, -3004, -3005, -3006, -3007, -3008, -3009, -3010, -3011, -3012, -3013, -3014, -3015, -3016, -3017, -3018, -3019, -3020
-4001, -4002, -4003, -4004, -4005, -4006, -4007, -4008, -4009, -4010, -4011, -4012, -4013, -4014, -4015, -4016, -4017, -4018, -4019, -4020
-5001, -5002, -5003, -5004, -5005, -5006, -5007, -5008, -5009, -5010, -5011, -5012, -5013, -5014, -5015, -5016, -5017, -5018, -5019, -5020
-6001, -6002, -6003, -6004, -6005, -6006, -6007, -6008, -6009, -6010, -6011, -6012, -6013, -6014, -6015, -6016, -6017, -6018, -6019, -6020
-7001, -7002, -7003, -7004, -7005, -7006, -7007, -7008, -7009, -7010, -7011, -7012, -7013, -7014, -7015, -7016, -7017, -7018, -7019, -7020
-8001, -8002, -8003, -8004, -8005, -8006, -8007, -8008, -8009, -8010, -8011, -8012, -8013, -8014, -8015, -8016, -8017, -8018, -8019, -8020
-9001, -9002, -9003, -9004, -9005, -9006, -9007, -9008, -9009, -9010, -9011, -9012, -9013, -9014, -9015, -9016, -9017, -9018, -9019, -9020
-10001, -10002, -10003, -10004, -10005, -10006, -10007, -10008, -10009, -10010, -10011, -10012, -10013, -10014, -10015, -10016, -10017, -10018, -10019, -10020
-11001, -11002, -11003, -11004, -11005, -11006, -11007, -11008, -11009, -11010, -11011, -11012, -11013, -11014, -11015, -11016, -11017, -11018, -11019, -11020
-12001, -12002, -12003, -12004, -12005, -12006, -12007, -12008, -12009, -12010, -12011, -12012, -12013, -12014, -12015, -12016, -12017, -12018, -12019, -12020
-13001, -13002, -13003, -13004, -13005, -13006, -13007, -13008, -13009, -13010, -13011, -13012, -13013, -13014, -13015, -13016, -13017, -13018, -13019, -13020
-14001, -14002, -14003, -14004, -14005, -14006, -14007, -14008, -14009, -14010, -14011, -14012, -14013, -14014, -14015, -14016, -14017, -14018, -14019, -14020
-15001, -15002, -15003, -15004, -15005, -15006, -15007, -15008, -15009, -15010, -15011, -15012, -15013, -15014, -15015, -15016, -15017, -15018, -15019, -15020
-16001, -16002, -16003, -16004, -16005, -16006, -16007, -16008, -16009, -16010, -16011, -16012, -16013, -16014, -16015, -16016, -16017, -16018, -16019, -16020
-17001, -17002, -17003, -17004, -17005, -17006, -17007, -17008, -17009, -17010, -17011, -17012, -17013, -17014, -17015, -17016, -17017, -17018, -17019, -17020
-18001, -18002, -18003, -18004, -18005, -18006, -18007, -18008, -18009, -18010, -18011, -18012, -18013, -18014, -18015, -18016, -18017, -18018, -18019, -18020
-19001, -19002, -19003, -19004, -19005, -19006, -19007, -19008, -19009, -19010, -19011, -19012, -19013, -19014, -19015, -19016, -19017, -19018, -19019, -19020

No. of Block Read = 77

No. of Block Written = 0

No. of Block Accessed = 77

> PRINT MATRIX VERYBIGANTI

1, 1001, 2001, 3001, 4001, 5001, 6001, 7001, 8001, 9001, 10001, 11001, 12001, 13001, 14001, 15001, 16001, 17001, 18001, 19001
2, 1002, 2002, 3002, 4002, 5002, 6002, 7002, 8002, 9002, 10002, 11002, 12002, 13002, 14002, 15002, 16002, 17002, 18002, 19002
3, 1003, 2003, 3003, 4003, 5003, 6003, 7003, 8003, 9003, 10003, 11003, 12003, 13003, 14003, 15003, 16003, 17003, 18003, 19003
4, 1004, 2004, 3004, 4004, 5004, 6004, 7004, 8004, 9004, 10004, 11004, 12004, 13004, 14004, 15004, 16004, 17004, 18004, 19004
5, 1005, 2005, 3005, 4005, 5005, 6005, 7005, 8005, 9005, 10005, 11005, 12005, 13005, 14005, 15005, 16005, 17005, 18005, 19005
6, 1006, 2006, 3006, 4006, 5006, 6006, 7006, 8006, 9006, 10006, 11006, 12006, 13006, 14006, 15006, 16006, 17006, 18006, 19006
7, 1007, 2007, 3007, 4007, 5007, 6007, 7007, 8007, 9007, 10007, 11007, 12007, 13007, 14007, 15007, 16007, 17007, 18007, 19007
8, 1008, 2008, 3008, 4008, 5008, 6008, 7008, 8008, 9008, 10008, 11008, 12008, 13008, 14008, 15008, 16008, 17008, 18008, 19008
9, 1009, 2009, 3009, 4009, 5009, 6009, 7009, 8009, 9009, 10009, 11009, 12009, 13009, 14009, 15009, 16009, 17009, 18009, 19009
10, 1010, 2010, 3010, 4010, 5010, 6010, 7010, 8010, 9010, 10010, 11010, 12010, 13010, 14010, 15010, 16010, 17010, 18010, 19010
11, 1011, 2011, 3011, 4011, 5011, 6011, 7011, 8011, 9011, 10011, 11011, 12011, 13011, 14011, 15011, 16011, 17011, 18011, 19011
12, 1012, 2012, 3012, 4012, 5012, 6012, 7012, 8012, 9012, 10012, 11012, 12012, 13012, 14012, 15012, 16012, 17012, 18012, 19012
13, 1013, 2013, 3013, 4013, 5013, 6013, 7013, 8013, 9013, 10013, 11013, 12013, 13013, 14013, 15013, 16013, 17013, 18013, 19013
14, 1014, 2014, 3014, 4014, 5014, 6014, 7014, 8014, 9014, 10014, 11014, 12014, 13014, 14014, 15014, 16014, 17014, 18014, 19014
15, 1015, 2015, 3015, 4015, 5015, 6015, 7015, 8015, 9015, 10015, 11015, 12015, 13015, 14015, 15015, 16015, 17015, 18015, 19015
16, 1016, 2016, 3016, 4016, 5016, 6016, 7016, 8016, 9016, 10016, 11016, 12016, 13016, 14016, 15016, 16016, 17016, 18016, 19016
17, 1017, 2017, 3017, 4017, 5017, 6017, 7017, 8017, 9017, 10017, 11017, 12017, 13017, 14017, 15017, 16017, 17017, 18017, 19017
18, 1018, 2018, 3018, 4018, 5018, 6018, 7018, 8018, 9018, 10018, 11018, 12018, 13018, 14018, 15018, 16018, 17018, 18018, 19018
19, 1019, 2019, 3019, 4019, 5019, 6019, 7019, 8019, 9019, 10019, 11019, 12019, 13019, 14019, 15019, 16019, 17019, 18019, 19019
20, 1020, 2020, 3020, 4020, 5020, 6020, 7020, 8020, 9020, 10020, 11020, 12020, 13020, 14020, 15020, 16020, 17020, 18020, 19020

No. of Block Read = 77

No. of Block Written = 0

No. of Block Accessed = 77

> ROTATE VERYBIG

Matrix VERYBIG rotated successfully.

No. of Block Read = 505995

No. of Block Written = 505995

No. of Block Accessed = 1011990

> LOAD MATRIX BIGM

Loaded Matrix BIGM. Dimension: 25 x 25

No. of Block Read = 0

No. of Block Written = 3

No. of Block Accessed = 3

> LOAD MATRIX BIGM_ANTI

Loaded Matrix BIGM_ANTI. Dimension: 25 x 25

No. of Block Read = 0

No. of Block Written = 3

No. of Block Accessed = 3

> PRINT MATRIX BIGM

17, 27, 84, 77, 81, 95, 93, 57, 24, 36, 22, 91, 2, 49, 57, 99, 77, 5, 6, 57
99, 53, 27, 15, 82, 81, 27, 67, 95, 72, 29, 56, 31, 28, 87, 97, 52, 72, 82, 84
95, 87, 24, 40, 28, 10, 80, 7, 4, 2, 74, 69, 76, 49, 61, 11, 44, 81, 71, 21
84, 47, 96, 60, 39, 30, 32, 89, 11, 27, 41, 44, 65, 68, 57, 36, 57, 21, 86, 14
75, 73, 31, 85, 91, 91, 27, 77, 4, 12, 26, 99, 49, 81, 50, 4, 20, 56, 66, 83
54, 28, 71, 53, 68, 2, 98, 27, 79, 9, 66, 95, 53, 48, 12, 42, 94, 64, 12, 72
37, 26, 9, 6, 94, 15, 25, 20, 38, 7, 42, 22, 70, 71, 1, 86, 81, 20, 71, 64
89, 54, 67, 83, 65, 78, 32, 41, 25, 48, 28, 44, 14, 74, 79, 32, 84, 1, 7, 61
16, 37, 19, 68, 40, 68, 91, 60, 8, 76, 27, 50, 20, 94, 2, 41, 70, 15, 70, 10
54, 53, 26, 100, 46, 97, 77, 10, 45, 62, 84, 26, 87, 36, 71, 81, 79, 84, 55, 17
47, 71, 83, 100, 8, 16, 56, 36, 24, 33, 62, 77, 31, 66, 91, 76, 9, 20, 25, 75
78, 37, 50, 25, 39, 8, 91, 5, 51, 4, 6, 71, 37, 94, 20, 14, 53, 19, 15, 75
97, 51, 6, 19, 66, 93, 23, 32, 97, 100, 16, 50, 87, 23, 64, 14, 69, 92, 48, 45
82, 96, 92, 33, 74, 64, 92, 60, 59, 40, 1, 56, 13, 91, 12, 50, 66, 32, 30, 62
5, 35, 50, 92, 32, 27, 89, 81, 58, 46, 50, 72, 76, 51, 94, 13, 75, 63, 30, 90
43, 42, 4, 50, 1, 26, 42, 60, 86, 80, 21, 15, 49, 74, 13, 84, 64, 75, 89, 91
99, 98, 87, 28, 67, 22, 6, 85, 17, 7, 25, 6, 94, 47, 36, 73, 73, 4, 64, 72
79, 72, 3, 6, 16, 95, 64, 40, 3, 71, 59, 70, 8, 9, 99, 25, 15, 78, 39, 91
66, 58, 39, 26, 77, 33, 34, 38, 11, 13, 62, 74, 69, 10, 54, 98, 19, 2, 35, 86
53, 22, 11, 33, 54, 2, 55, 23, 36, 83, 83, 58, 99, 38, 72, 73, 98, 12, 56, 99

No. of Block Read = 2

No. of Block Written = 0

No. of Block Accessed = 2

> PRINT MATRIX BIGM_ANTI

-17, -99, -95, -84, -75, -54, -37, -89, -16, -54, -47, -78, -97, -82, -5, -43, -99, -79, -66, -53
-27, -53, -87, -47, -73, -28, -26, -54, -37, -53, -71, -37, -51, -96, -35, -42, -98, -72, -58, -22
-84, -27, -24, -96, -31, -71, -9, -67, -19, -26, -83, -50, -6, -92, -50, -4, -87, -3, -39, -11
-77, -15, -40, -60, -85, -53, -6, -83, -68, -100, -100, -25, -19, -33, -92, -50, -28, -6, -26, -33
-81, -82, -28, -39, -91, -68, -94, -65, -40, -46, -8, -39, -66, -74, -32, -1, -67, -16, -77, -54
-95, -81, -10, -30, -91, -2, -15, -78, -68, -97, -16, -8, -93, -64, -27, -26, -22, -95, -33, -2
-93, -27, -80, -32, -27, -98, -25, -32, -91, -77, -56, -91, -23, -92, -89, -42, -6, -64, -34, -55
-57, -67, -7, -89, -77, -27, -20, -41, -60, -10, -36, -5, -32, -60, -81, -60, -85, -40, -38, -23
-24, -95, -4, -11, -4, -79, -38, -25, -8, -45, -24, -51, -97, -59, -58, -86, -17, -3, -11, -36
-36, -72, -2, -27, -12, -9, -7, -48, -76, -62, -33, -4, -100, -40, -46, -80, -7, -71, -13, -83
-22, -29, -74, -41, -26, -66, -42, -28, -27, -84, -62, -6, -16, -1, -50, -21, -25, -59, -62, -83
-91, -56, -69, -44, -99, -95, -22, -44, -50, -26, -77, -71, -50, -56, -72, -15, -6, -70, -74, -58
-2, -31, -76, -65, -49, -53, -70, -14, -20, -87, -31, -37, -87, -13, -76, -49, -94, -8, -69, -99
-49, -28, -49, -68, -81, -48, -71, -74, -94, -36, -66, -94, -23, -91, -51, -74, -47, -9, -10, -38
-57, -87, -61, -57, -50, -12, -1, -79, -2, -71, -91, -20, -64, -12, -94, -13, -36, -99, -54, -72
-99, -97, -11, -36, -4, -42, -86, -32, -41, -81, -76, -14, -14, -50, -13, -84, -73, -25, -98, -73
-77, -52, -44, -57, -20, -94, -81, -84, -70, -79, -9, -53, -69, -66, -75, -64, -73, -15, -19, -98
-5, -72, -81, -21, -56, -64, -20, -1, -15, -84, -20, -19, -92, -32, -63, -75, -4, -78, -2, -12
-6, -82, -71, -86, -66, -12, -71, -7, -70, -55, -25, -15, -48, -30, -30, -89, -64, -39, -35, -56
-57, -84, -21, -14, -83, -72, -64, -61, -10, -17, -75, -75, -45, -62, -90, -91, -72, -91, -86, -99

No. of Block Read = 2

No. of Block Written = 0

No. of Block Accessed = 2

> CROSSTRANSPOSE BIGM BIGM_ANTI

CROSSTRANSPOSE completed: BIGM now holds BIGM_ANTI's transpose and vice versa.

No. of Block Read = 52

No. of Block Written = 52

No. of Block Accessed = 104

> PRINT MATRIX BIGM

-17, -27, -84, -77, -81, -95, -93, -57, -24, -36, -22, -91, -2, -49, -57, -99, -77, -5, -6, -57
-99, -53, -27, -15, -82, -81, -27, -67, -95, -72, -29, -56, -31, -28, -87, -97, -52, -72, -82, -84
-95, -87, -24, -40, -28, -10, -80, -7, -4, -2, -74, -69, -76, -49, -61, -11, -44, -81, -71, -21
-84, -47, -96, -60, -39, -30, -32, -89, -11, -27, -41, -44, -65, -68, -57, -36, -57, -21, -86, -14
-75, -73, -31, -85, -91, -91, -27, -77, -4, -12, -26, -99, -49, -81, -50, -4, -20, -56, -66, -83
-54, -28, -71, -53, -68, -2, -98, -27, -79, -9, -66, -95, -53, -48, -12, -42, -94, -64, -12, -72
-37, -26, -9, -6, -94, -15, -25, -20, -38, -7, -42, -22, -70, -71, -1, -86, -81, -20, -71, -64
-89, -54, -67, -83, -65, -78, -32, -41, -25, -48, -28, -44, -14, -74, -79, -32, -84, -1, -7, -61
-16, -37, -19, -68, -40, -68, -91, -60, -8, -76, -27, -50, -20, -94, -2, -41, -70, -15, -70, -10
-54, -53, -26, -100, -46, -97, -77, -10, -45, -62, -84, -26, -87, -36, -71, -81, -79, -84, -55, -17
-47, -71, -83, -100, -8, -16, -56, -36, -24, -33, -62, -77, -31, -66, -91, -76, -9, -20, -25, -75
-78, -37, -50, -25, -39, -8, -91, -5, -51, -4, -6, -71, -37, -94, -20, -14, -53, -19, -15, -75
-97, -51, -6, -19, -66, -93, -23, -32, -97, -100, -16, -50, -87, -23, -64, -14, -69, -92, -48, -45
-82, -96, -92, -33, -74, -64, -92, -60, -59, -40, -1, -56, -13, -91, -12, -50, -66, -32, -30, -62
-5, -35, -50, -92, -32, -27, -89, -81, -58, -46, -50, -72, -76, -51, -94, -13, -75, -63, -30, -90
-43, -42, -4, -50, -1, -26, -42, -60, -86, -80, -21, -15, -49, -74, -13, -84, -64, -75, -89, -91
-99, -98, -87, -28, -67, -22, -6, -85, -17, -7, -25, -6, -94, -47, -36, -73, -73, -4, -64, -72
-79, -72, -3, -6, -16, -95, -64, -40, -3, -71, -59, -70, -8, -9, -99, -25, -15, -78, -39, -91
-66, -58, -39, -26, -77, -33, -34, -38, -11, -13, -62, -74, -69, -10, -54, -98, -19, -2, -35, -86
-53, -22, -11, -33, -54, -2, -55, -23, -36, -83, -83, -58, -99, -38, -72, -73, -98, -12, -56, -99

No. of Block Read = 2

No. of Block Written = 0

No. of Block Accessed = 2

> PRINT MATRIX BIGM_ANTI

17, 99, 95, 84, 75, 54, 37, 89, 16, 54, 47, 78, 97, 82, 5, 43, 99, 79, 66, 53
27, 53, 87, 47, 73, 28, 26, 54, 37, 53, 71, 37, 51, 96, 35, 42, 98, 72, 58, 22
84, 27, 24, 96, 31, 71, 9, 67, 19, 26, 83, 50, 6, 92, 50, 4, 87, 3, 39, 11
77, 15, 40, 60, 85, 53, 6, 83, 68, 100, 100, 25, 19, 33, 92, 50, 28, 6, 26, 33
81, 82, 28, 39, 91, 68, 94, 65, 40, 46, 8, 39, 66, 74, 32, 1, 67, 16, 77, 54
95, 81, 10, 30, 91, 2, 15, 78, 68, 97, 16, 8, 93, 64, 27, 26, 22, 95, 33, 2
93, 27, 80, 32, 27, 98, 25, 32, 91, 77, 56, 91, 23, 92, 89, 42, 6, 64, 34, 55
57, 67, 7, 89, 77, 27, 20, 41, 60, 10, 36, 5, 32, 60, 81, 60, 85, 40, 38, 23
24, 95, 4, 11, 4, 79, 38, 25, 8, 45, 24, 51, 97, 59, 58, 86, 17, 3, 11, 36
36, 72, 2, 27, 12, 9, 7, 48, 76, 62, 33, 4, 100, 40, 46, 80, 7, 71, 13, 83
22, 29, 74, 41, 26, 66, 42, 28, 27, 84, 62, 6, 16, 1, 50, 21, 25, 59, 62, 83
91, 56, 69, 44, 99, 95, 22, 44, 50, 26, 77, 71, 50, 56, 72, 15, 6, 70, 74, 58
2, 31, 76, 65, 49, 53, 70, 14, 20, 87, 31, 37, 87, 13, 76, 49, 94, 8, 69, 99
49, 28, 49, 68, 81, 48, 71, 74, 94, 36, 66, 94, 23, 91, 51, 74, 47, 9, 10, 38
57, 87, 61, 57, 50, 12, 1, 79, 2, 71, 91, 20, 64, 12, 94, 13, 36, 99, 54, 72
99, 97, 11, 36, 4, 42, 86, 32, 41, 81, 76, 14, 14, 50, 13, 84, 73, 25, 98, 73
77, 52, 44, 57, 20, 94, 81, 84, 70, 79, 9, 53, 69, 66, 75, 64, 73, 15, 19, 98
5, 72, 81, 21, 56, 64, 20, 1, 15, 84, 20, 19, 92, 32, 63, 75, 4, 78, 2, 12
6, 82, 71, 86, 66, 12, 71, 7, 70, 55, 25, 15, 48, 30, 30, 89, 64, 39, 35, 56
57, 84, 21, 14, 83, 72, 64, 61, 10, 17, 75, 75, 45, 62, 90, 91, 72, 91, 86, 99

No. of Block Read = 2

No. of Block Written = 0

No. of Block Accessed = 2

> EXPORT MATRIX BIGM

Matrix 'BIGM' updated at ../data/BIGM.csv

No. of Block Read = 3

No. of Block Written = 0

No. of Block Accessed = 3

> QUIT

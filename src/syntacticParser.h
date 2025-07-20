#pragma once
#include "matrixCatalogue.h"
#include "tableCatalogue.h"


using namespace std;

enum QueryType {
    CLEAR,
    CROSS,
    DISTINCT,
    EXPORT,
    INDEX,
    INSERT,
    JOIN,
    LIST,
    LOAD,
    PRINT,
    PROJECTION,
    RENAME,
    SELECTION,
    SORT,
    SOURCE,
    LOADMATRIX,
    PRINTMATRIX,
    EXPORTMATRIX,
    CHECKANTISYM,
    ROTATE,
    CROSSTRANSPOSE,
    ORDERBY,
    GROUPBY,
    SEARCH,
    DELETE,
    UPDATE,
    UNDETERMINED
};

enum BinaryOperator {
    LESS_THAN,
    GREATER_THAN,
    LEQ,
    GEQ,
    EQUAL,
    NOT_EQUAL,
    NO_BINOP_CLAUSE
};

enum AggregateFunction
{
    MIN,
    MAX,
    SUM,
    AVG,
    CNT,
    NO_AGGREGATE
};


// enum SortingStrategy { ASC, DESC, NO_SORT_CLAUSE };

enum SelectType { COLUMN, INT_LITERAL, NO_SELECT_CLAUSE };

class ParsedQuery {

  public:
    QueryType queryType = UNDETERMINED;

    vector<string> sortColumnNames;
    vector<SortingStrategy> sortingStrategies;

    string orderByRelationName = "";

    string clearRelationName = "";

    string crossResultRelationName = "";
    string crossFirstRelationName = "";
    string crossSecondRelationName = "";
    string loadMatrixName = "";
    string printMatrixName = "";
    string distinctResultRelationName = "";
    string distinctRelationName = "";

    string crossTransposeMatrixName1 = "";
    string crossTransposeMatrixName2 = "";
    string rotateMatrixName = "";
    string checkAntisymMatrixName1 = "";
    string checkAntisymMatrixName2 = "";

    string exportMatrixName = "";

    string exportRelationName = "";

    IndexingStrategy indexingStrategy = NOTHING;
    string indexColumnName = "";
    string indexRelationName = "";

    BinaryOperator joinBinaryOperator = NO_BINOP_CLAUSE;
    string joinResultRelationName = "";
    string joinFirstRelationName = "";
    string joinSecondRelationName = "";
    string joinFirstColumnName = "";
    string joinSecondColumnName = "";

    string loadRelationName = "";

    string printRelationName = "";

    string projectionResultRelationName = "";
    vector<string> projectionColumnList;
    string projectionRelationName = "";

    string renameFromColumnName = "";
    string renameToColumnName = "";
    string renameRelationName = "";

    SelectType selectType = NO_SELECT_CLAUSE;
    BinaryOperator selectionBinaryOperator = NO_BINOP_CLAUSE;
    string selectionResultRelationName = "";
    string selectionRelationName = "";
    string selectionFirstColumnName = "";
    string selectionSecondColumnName = "";
    int selectionIntLiteral = 0;

    SortingStrategy sortingStrategy = NO_SORT_CLAUSE;
    string sortResultRelationName = "";
    string sortColumnName = "";
    string sortRelationName = "";

    SortingStrategy orderby_sort_strats = NO_SORT_CLAUSE;
    string orderby_res_table_name = "";
    string orderByColumnName = "";
    string orderby_table_name = "";

    BinaryOperator OP = NO_BINOP_CLAUSE;
    string groupby_res_table_name = "";
    string groupby_table_name = "";
    string groupByTableName = "";
    string groupby_val = "";
    string groupby_have_func_col = "";
    string groupby_return_func_col = "";
    string groupby_have_aggregate_col = "";
    string groupby_return_aggregate_col = "";
    AggregateFunction returnAggregateFunction = NO_AGGREGATE;
    AggregateFunction havingAggregateFunction = NO_AGGREGATE;

    string insertRelationName = "";
    vector<string> insertColumnNames;
    vector<int>    insertColumnValues;

    string updateColumnName = "";
    int updateValue;

    string sourceFileName = "";

    ParsedQuery();
    void clear();
};

bool syntacticParse();
bool syntacticParseCLEAR();
bool syntacticParseCROSS();
bool syntacticParseDISTINCT();
bool syntacticParseEXPORT();
bool syntacticParseINDEX();
bool syntacticParseJOIN();
bool syntacticParseLIST();
bool syntacticParseLOAD();
bool syntacticParsePRINT();
bool syntacticParsePROJECTION();
bool syntacticParseRENAME();
bool syntacticParseSELECTION();
bool syntacticParseSORT();
bool syntacticParseSOURCE();
bool syntacticParseLOADMATRIX();
bool syntacticParsePRINTMATRIX();
bool syntacticParseCHECKANTISYM();
bool syntacticParseROTATE();
bool syntacticParseCROSSTRANSPOSE();
bool syntacticParseEXPORTMATRIX();
bool syntacticParseORDERBY();
bool syntacticParseGROUPBY();
bool syntacticParseSEARCH();
bool syntacticParseDELETE();
bool syntacticParseUPDATE();
bool syntacticParseINSERT();

bool isFileExists(string tableName);
bool isQueryFile(string fileName);

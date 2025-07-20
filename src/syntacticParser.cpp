#include "global.h"

bool syntacticParse()
{
    logger.log("syntacticParse");
    string possibleQueryType = tokenizedQuery[0];

    if (tokenizedQuery.size() < 2)
    {
        cout << "SYNTAX ERROR (query too small)" << endl;
        return false;
    }
    if (possibleQueryType == "LOAD") {
        if (tokenizedQuery.size() >= 2 && tokenizedQuery[1] == "MATRIX") {
            return syntacticParseLOADMATRIX();
        } else {
            return syntacticParseLOAD();
        }
    }

    if (possibleQueryType == "PRINT") {
        if (tokenizedQuery.size() >= 2 && tokenizedQuery[1] == "MATRIX") {
            return syntacticParsePRINTMATRIX();
        } else {
            return syntacticParsePRINT();
        }
    }

    if (possibleQueryType == "EXPORT") {
        if (tokenizedQuery.size() >= 2 && tokenizedQuery[1] == "MATRIX") {
            return syntacticParseEXPORTMATRIX();
        } else {
            return syntacticParseEXPORT();
        }
    }

    if (possibleQueryType == "CHECKANTISYM") {
        return syntacticParseCHECKANTISYM();
    }

    if (possibleQueryType == "ROTATE") {
        return syntacticParseROTATE();
    }

    if (possibleQueryType == "CROSSTRANSPOSE") {
        return syntacticParseCROSSTRANSPOSE();
    }


    if (possibleQueryType == "CLEAR")
        return syntacticParseCLEAR();
    else if (possibleQueryType == "INDEX")
        return syntacticParseINDEX();
    else if (possibleQueryType == "LIST")
        return syntacticParseLIST();
    // else if (possibleQueryType == "LOAD")
    //     return syntacticParseLOAD();
    // else if (possibleQueryType == "PRINT")
    //     return syntacticParsePRINT();
    else if (possibleQueryType == "RENAME")
        return syntacticParseRENAME();
    // else if(possibleQueryType == "EXPORT")
    //     return syntacticParseEXPORT();
    else if(possibleQueryType == "SOURCE")
        return syntacticParseSOURCE();
    else if (possibleQueryType == "SORT")
            return syntacticParseSORT();
    else if (possibleQueryType == "INSERT")
            return syntacticParseINSERT();
    else if (possibleQueryType == "DELETE")
            return syntacticParseDELETE();
    else if (possibleQueryType == "UPDATE")
            return syntacticParseUPDATE();
    else
    {
        string resultantRelationName = possibleQueryType;
        // cout << tokenizedQuery[0] << tokenizedQuery[1]<<tokenizedQuery.size()<< endl;
        if (tokenizedQuery[1] != "<-" || tokenizedQuery.size() < 3)
        {
            cout << "SYNTAX ERROR (wrong)" << endl;
            return false;
        }
        possibleQueryType = tokenizedQuery[2];
        if(possibleQueryType == "ORDER")
            return syntacticParseORDERBY();
        else if (possibleQueryType == "GROUP")
            return syntacticParseGROUPBY();
        else if (possibleQueryType == "PROJECT")
            return syntacticParsePROJECTION();
        else if (possibleQueryType == "SELECT")
            return syntacticParseSELECTION();
        else if (possibleQueryType == "JOIN")
            return syntacticParseJOIN();
        else if (possibleQueryType == "CROSS")
            return syntacticParseCROSS();
        else if (possibleQueryType == "DISTINCT")
            return syntacticParseDISTINCT();
        else if (possibleQueryType == "SEARCH")
            return syntacticParseSEARCH();
        else if (possibleQueryType == "DELETE")
            return syntacticParseDELETE();
        else if (possibleQueryType == "UPDATE")
            return syntacticParseUPDATE();
        else
        {
            cout << "SYNTAX ERROR (no matches)" << endl;
            return false;
        }
    }
    return false;
}

ParsedQuery::ParsedQuery()
{
}

void ParsedQuery::clear()
{
    logger.log("ParseQuery::clear");
    this->queryType = UNDETERMINED;

    this->clearRelationName = "";
    this->loadMatrixName = "";
    this->printMatrixName = "";
    this->crossResultRelationName = "";
    this->crossFirstRelationName = "";
    this->crossSecondRelationName = "";

    this->checkAntisymMatrixName1 = "";
    this->checkAntisymMatrixName2 = "";

    this->crossTransposeMatrixName1 = "";
    this->crossTransposeMatrixName2 = "";

    this->rotateMatrixName = "";

    this->distinctResultRelationName = "";
    this->distinctRelationName = "";

    this->exportRelationName = "";

    this->indexingStrategy = NOTHING;
    this->indexColumnName = "";
    this->indexRelationName = "";

    this->joinBinaryOperator = NO_BINOP_CLAUSE;
    this->joinResultRelationName = "";
    this->joinFirstRelationName = "";
    this->joinSecondRelationName = "";
    this->joinFirstColumnName = "";
    this->joinSecondColumnName = "";

    this->loadRelationName = "";

    this->printRelationName = "";

    this->projectionResultRelationName = "";
    this->projectionColumnList.clear();
    this->projectionRelationName = "";

    this->renameFromColumnName = "";
    this->renameToColumnName = "";
    this->renameRelationName = "";

    this->exportMatrixName = "";

    this->selectType = NO_SELECT_CLAUSE;
    this->selectionBinaryOperator = NO_BINOP_CLAUSE;
    this->selectionResultRelationName = "";
    this->selectionRelationName = "";
    this->selectionFirstColumnName = "";
    this->selectionSecondColumnName = "";
    this->selectionIntLiteral = 0;

    this->sortingStrategy = NO_SORT_CLAUSE;
    this->sortResultRelationName = "";
    this->sortColumnName = "";
    this->sortRelationName = "";

    this->orderby_sort_strats = NO_SORT_CLAUSE;
    this->orderby_res_table_name = "";
    this->orderByColumnName = "";
    this->orderby_table_name = "";

    this->groupby_res_table_name = "";
    this->groupby_table_name = "";
    this->groupby_val = "";
    this->groupby_have_func_col = "";
    this->groupby_return_func_col = "";
    this->OP = NO_BINOP_CLAUSE;
    this->havingAggregateFunction = NO_AGGREGATE;
    this->returnAggregateFunction = NO_AGGREGATE;
    this->groupby_have_aggregate_col = "";
    this->groupby_return_aggregate_col = "";

    this->insertRelationName = "";
    this->insertColumnNames.clear();
    this->insertColumnValues.clear();

    this->sourceFileName = "";
}

/**
 * @brief Checks to see if source file exists. Called when LOAD command is
 * invoked.
 *
 * @param tableName 
 * @return true 
 * @return false 
 */
bool isFileExists(string tableName)
{
    string fileName = "../data/" + tableName + ".csv";
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

/**
 * @brief Checks to see if source file exists. Called when SOURCE command is
 * invoked.
 *
 * @param tableName 
 * @return true 
 * @return false 
 */
bool isQueryFile(string fileName){
    fileName = "../data/" + fileName + ".ra";
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

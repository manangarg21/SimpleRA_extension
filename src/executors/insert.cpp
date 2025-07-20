#include "global.h"
#include <iostream>

bool syntacticParseINSERT() {
    logger.log("syntacticParseINSERT");
    // INSERT INTO table_name ( col1 = val1, col2 = val2, … )
    if (tokenizedQuery.size() < 4 || tokenizedQuery[1] != "INTO") {
        cout << "SYNTAX ERROR 1" << endl; //TBD
        return false;
    }
    parsedQuery.queryType          = INSERT;
    parsedQuery.insertRelationName = tokenizedQuery[2];
    parsedQuery.insertColumnNames.clear();
    parsedQuery.insertColumnValues.clear();

    // Parse "( col = val , … )"
    //TBD: MODIFY
    for (size_t i = 3; i + 2 < tokenizedQuery.size(); ++i) {
        string tok = tokenizedQuery[i];
        if (tok == "(" || tok == ",") continue;
        // strip parentheses if attached
        if (tok.front() == '(') tok = tok.substr(1);
        if (tok.back()  == ')') tok.pop_back();

        string col = tok;
        if (tokenizedQuery[i+1] != "=") {
            cout << "SYNTAX ERROR 2" << endl; //TBD
            return false;
        }
        string valTok = tokenizedQuery[i+2];
        if (valTok.back() == ')') valTok.pop_back();
        int val = stoi(valTok);

        parsedQuery.insertColumnNames.push_back(col);
        parsedQuery.insertColumnValues.push_back(val);
        i += 2;
    }

    if (parsedQuery.insertColumnNames.empty()) {
        cout << "SYNTAX ERROR 3" << endl; //TBD
        return false;
    }

    // cout << "Reach here: Line: " << __LINE__ << endl; //TBD

    return true;
}

bool semanticParseINSERT() 
{
    logger.log("semanticParseINSERT");
    string tbl = parsedQuery.insertRelationName;
    if (!tableCatalogue.isTable(tbl)) {
        cout << "SEMANTIC ERROR: Relation doesn’t exist" << endl;
        return false;
    }
    // validate each column
    for (auto &col : parsedQuery.insertColumnNames) {
        if (!tableCatalogue.isColumnFromTable(col, tbl)) {
            cout << "SEMANTIC ERROR: Column “" << col << "” doesn’t exist" << endl;
            return false;
        }
    }

    // cout << "Reach here: Line: " << __LINE__ << endl;
    return true;
}

void executeINSERT() {
    logger.log("executeINSERT");
    string tbl = parsedQuery.insertRelationName;
    Table *T = tableCatalogue.getTable(tbl);
    if (!T) {
        cout << "ERROR: table not found\n";
        return;
    }

    // cout << "Reach here: Line: " << __LINE__ << endl;

    // Build the new row, defaulting all columns to 0
    vector<int> newRow(T->columnCount, 0);
    for (size_t i = 0; i < parsedQuery.insertColumnNames.size(); ++i) {
        const string &col = parsedQuery.insertColumnNames[i];
        int val           = parsedQuery.insertColumnValues[i];
        int idx           = T->getColumnIndex(col);
        newRow[idx]       = val;
    }

    // cout << "Reach here: Line: " << __LINE__ << endl;

    T->appendRowToTable(newRow);

    T->markAllColsIndexDirty();
    
}
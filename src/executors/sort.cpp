#include"global.h"
/**
 * @brief File contains method to process SORT commands.
 * 
 */
 bool syntacticParseSORT() {
    logger.log("syntacticParseSORT");

    // Check basic structure
    // Query should have at least: SORT <relationName> BY <columns> IN <orders>
    if (tokenizedQuery.size() < 6) {
        cout << "SYNTAX ERROR (Incomplete Query)" << endl;
        return false;
    }

    parsedQuery.queryType = SORT;
    parsedQuery.sortRelationName = tokenizedQuery[1];

    // Find the index of "BY" keyword
    int byIndex = -1;
    for (int i = 2; i < tokenizedQuery.size(); ++i) {
        if (tokenizedQuery[i] == "BY") {
            byIndex = i;
            break;
        }
    }

    // Find the index of "IN" keyword
    int inIndex = -1;
    for (int i = byIndex + 1; i < tokenizedQuery.size(); ++i) {
        if (tokenizedQuery[i] == "IN") {
            inIndex = i;
            break;
        }
    }

    // Check if "BY" and "IN" keywords are found and in the correct order
    if (byIndex == -1 || inIndex == -1 || byIndex >= inIndex - 1) {
        cout << "SYNTAX ERROR (Incorrect SORT query structure. Expected 'SORT <relation> BY <columns> IN <orders>')" << endl;
        return false;
    }

    // Extract columns after "BY"
    vector<string> columns;
    if (byIndex + 1 < inIndex) {
        stringstream colStream;
        for (int i = byIndex + 1; i < inIndex; ++i) {
            colStream << tokenizedQuery[i];
            if (i < inIndex - 1) {
                colStream << ",";
            }
        }
        string column;
        while (getline(colStream, column, ',')) {
            columns.push_back(column);
        }
    } else {
        cout << "SYNTAX ERROR (No columns specified after 'BY')" << endl;
        return false;
    }


    // Extract orders after "IN"
    vector<string> orderTokens;
    for (int i = inIndex + 1; i < tokenizedQuery.size(); ++i) {
        orderTokens.push_back(tokenizedQuery[i]);
    }

    vector<SortingStrategy> orders;
    stringstream orderStream;
    for (const string& token : orderTokens) {
        orderStream << token;
        if (&token != &orderTokens.back()) {
            orderStream << ",";
        }
    }
    string orderStr;
    while (getline(orderStream, orderStr, ',')) {
        if (orderStr == "ASC") {
            orders.push_back(ASC);
        } else if (orderStr == "DESC") {
            orders.push_back(DESC);
        } else {
            cout << "SYNTAX ERROR (ASC and DESC only accepted in order list.)" << endl;
            return false;
        }
    }


    // Validate matching count
    if (columns.size() != orders.size()) {
        cout << "SYNTAX ERROR: Number of columns and sorting orders do not match" << endl;
        return false;
    }

    // Store extracted column names and sorting strategies
    parsedQuery.sortColumnNames = columns;
    parsedQuery.sortingStrategies = orders;
    return true;
}

bool semanticParseSORT(){
    logger.log("semanticParseSORT");


    if(!tableCatalogue.isTable(parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    for(const string& columnName: parsedQuery.sortColumnNames){
        if(!tableCatalogue.isColumnFromTable(columnName, parsedQuery.sortRelationName)){
            cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
            return false;
        }
    }

    return true;
}

void executeSORT(){
    logger.log("executeSORT");
    Table* table = tableCatalogue.getTable(parsedQuery.sortRelationName);
    table->sort(parsedQuery.sortColumnNames, parsedQuery.sortingStrategies);
    table->print();
    return;
}
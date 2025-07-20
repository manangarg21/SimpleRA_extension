#include"global.h"
/**
 * @brief File contains method to process to ORDERBY commands.
 * Result <- ORDER BY YearReleased ASC ON MOVIE
 */
 bool syntacticParseORDERBY() {
    logger.log("syntacticParseORDERBY");

    parsedQuery.queryType = ORDERBY;

    // Minimum tokens required: Result <- ORDER BY <column> <order> ON <relation> (8)
    if (tokenizedQuery.size() < 8) {
        cout << "SYNTAX ERROR (Incomplete ORDER BY query)" << endl;
        return false;
    }

    if (tokenizedQuery[1] != "<-") {
        cout << "SYNTAX ERROR (Expected '<-' after the result relation name)" << endl;
        return false;
    }
    parsedQuery.orderByRelationName = tokenizedQuery[0];

    if (tokenizedQuery[2] != "ORDER") {
        cout << "SYNTAX ERROR (Expected 'ORDER' keyword)" << endl;
        return false;
    }

    int byIndex = -1;
    for (int i = 3; i < tokenizedQuery.size(); ++i) {
        if (tokenizedQuery[i] == "BY") {
            byIndex = i;
            break;
        }
    }

    if (byIndex == -1) {
        cout << "SYNTAX ERROR ('BY' keyword missing)" << endl;
        return false;
    }

    int onIndex = -1;
    for (int i = byIndex + 1; i < tokenizedQuery.size(); ++i) {
        if (tokenizedQuery[i] == "ON") {
            onIndex = i;
            break;
        }
    }

    if (onIndex == -1) {
        cout << "SYNTAX ERROR ('ON' keyword missing)" << endl;
        return false;
    }

    if (onIndex <= byIndex + 1) {
        cout << "SYNTAX ERROR (No sorting criteria specified after 'BY')" << endl;
        return false;
    }

    // Extract sorting columns and strategies
    parsedQuery.sortColumnNames.clear();
    parsedQuery.sortingStrategies.clear();
    for (int i = byIndex + 1; i < onIndex; i += 2) {
        if (i + 1 >= onIndex) {
            cout << "SYNTAX ERROR (Missing sorting order for column '" << tokenizedQuery[i] << "')" << endl;
            return false;
        }
        parsedQuery.sortColumnNames.push_back(tokenizedQuery[i]);
        string order = tokenizedQuery[i + 1];
        if (order == "ASC") {
            parsedQuery.sortingStrategies.push_back(ASC);
        } else if (order == "DESC") {
            parsedQuery.sortingStrategies.push_back(DESC);
        } else {
            cout << "SYNTAX ERROR (Invalid sorting order '" << order << "'. Only ASC or DESC are allowed)" << endl;
            return false;
        }
    }

    if (onIndex + 1 >= tokenizedQuery.size()) {
        cout << "SYNTAX ERROR (Missing relation name after 'ON')" << endl;
        return false;
    }
    parsedQuery.sortRelationName = tokenizedQuery[onIndex + 1];

    return true;
}

bool semanticParseORDERBY(){
    logger.log("semanticParseORDERBY");


    if(!tableCatalogue.isTable(parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: ON Relation doesn't exist"<<endl;
        return false;
    }

    if(tableCatalogue.isTable(parsedQuery.orderByRelationName)){
        cout<<"SEMANTIC ERROR: Relation already exist"<<endl;
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

void executeORDERBY(){
    logger.log("executeORDERBY");

    bufferManager.clear();
    Table table = *tableCatalogue.getTable(parsedQuery.sortRelationName);

    Table* resultantTable = new Table(parsedQuery.orderByRelationName, table.columns);
    Cursor cursor = table.getCursor();
    vector<int> row = cursor.getNext();

    while (!row.empty())
    {
        resultantTable->writeRow<int>(row);
        row = cursor.getNext();
    }
    if(resultantTable->blockify()){
        tableCatalogue.insertTable(resultantTable);
        resultantTable->sort(parsedQuery.sortColumnNames, parsedQuery.sortingStrategies);
        bufferManager.clear();
        resultantTable->print();
    }
    return;
}
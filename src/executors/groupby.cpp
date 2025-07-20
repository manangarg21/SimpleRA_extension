#include "global.h"
/**
 * @brief
 * SYNTAX: <new_table> <- GROUP BY <grouping_attribute> FROM <table_name> HAVING <aggregate(attribute)> <bin_op> <attribute_value> RETURN <aggregate_func(attribute)>
 */
bool syntacticParseGROUPBY()
{
    logger.log("syntacticParseGROUPBY");
    if (tokenizedQuery.size() != 13 || tokenizedQuery[3] != "BY" || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "HAVING" || tokenizedQuery[11] != "RETURN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = GROUPBY;
    parsedQuery.groupby_res_table_name = tokenizedQuery[0];
    parsedQuery.groupby_table_name = tokenizedQuery[4];
    parsedQuery.groupByTableName = tokenizedQuery[6];
    parsedQuery.groupby_val = tokenizedQuery[10];
    
    parsedQuery.groupby_have_func_col = tokenizedQuery[8];
    parsedQuery.groupby_return_func_col = tokenizedQuery[12];

    string OP = tokenizedQuery[9];
    if (OP == "<")
        parsedQuery.OP = LESS_THAN;
    else if (OP == ">")
        parsedQuery.OP = GREATER_THAN;
    else if (OP == "<=")
        parsedQuery.OP = LEQ;
    else if (OP == ">=")
        parsedQuery.OP = GEQ;
    else if (OP == "==")
        parsedQuery.OP = EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    string groupby_have_func_col = tokenizedQuery[8];
    if (groupby_have_func_col.substr(0, 3) == "MAX")
        parsedQuery.havingAggregateFunction = MAX;
    else if (groupby_have_func_col.substr(0, 3) == "MIN")
        parsedQuery.havingAggregateFunction = MIN;
    else if (groupby_have_func_col.substr(0, 5) == "COUNT")
    {
        // cout << 123 << endl;
        parsedQuery.havingAggregateFunction = CNT;
    }
    else if (groupby_have_func_col.substr(0, 3) == "SUM")
        parsedQuery.havingAggregateFunction = SUM;
    else if (groupby_have_func_col.substr(0, 3) == "AVG")
        parsedQuery.havingAggregateFunction = AVG;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    // Extract the aggregate column name depending on the function
    if(parsedQuery.havingAggregateFunction == CNT)
        // For "COUNT(column)" the token length is at least 8, so remove "COUNT(" at the beginning and ")" at the end.
        parsedQuery.groupby_have_aggregate_col = groupby_have_func_col.substr(6, groupby_have_func_col.size() - 7);
    else
        // For functions with three-letter names (e.g. MAX, MIN, SUM, AVG), remove the first 4 chars ("MAX(") and the last char (")")
        parsedQuery.groupby_have_aggregate_col = groupby_have_func_col.substr(4, groupby_have_func_col.size() - 5);
    cout << parsedQuery.groupby_have_aggregate_col << endl;

    string groupby_return_func_col = tokenizedQuery[12];
    if (groupby_return_func_col.substr(0, 3) == "MAX")
        parsedQuery.returnAggregateFunction = MAX;
    else if (groupby_return_func_col.substr(0, 3) == "MIN")
        parsedQuery.returnAggregateFunction = MIN;
    else if (groupby_return_func_col.substr(0, 5) == "COUNT")
    {
        parsedQuery.returnAggregateFunction = CNT;
    }
    else if (groupby_return_func_col.substr(0, 3) == "SUM")
        parsedQuery.returnAggregateFunction = SUM;
    else if (groupby_return_func_col.substr(0, 3) == "AVG")
        parsedQuery.returnAggregateFunction = AVG;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    if(parsedQuery.returnAggregateFunction == CNT)
        parsedQuery.groupby_return_aggregate_col = groupby_return_func_col.substr(6, groupby_return_func_col.size() - 7);
    else
        parsedQuery.groupby_return_aggregate_col = groupby_return_func_col.substr(4, groupby_return_func_col.size() - 5);
    cout << parsedQuery.groupby_return_aggregate_col << endl;
    return true;
}

bool semanticParseGROUPBY()
{
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.groupby_res_table_name))
    {
        cout << "SEMANTIC ERROR: Resultant table already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.groupByTableName))
    {
        cout << "SEMANTIC ERROR: Table doesn't exist" << endl;
        return false;
    }

    Table *table = tableCatalogue.getTable(parsedQuery.groupByTableName);
    if (!table->isColumn(parsedQuery.groupby_table_name))
    {
        cout << "SEMANTIC ERROR: Grouping attribute not present in table" << endl;
        return false;
    }

    if (!table->isColumn(parsedQuery.groupby_have_aggregate_col))
    {
        cout << "SEMANTIC ERROR: Having aggregate column not present in table" << endl;
        return false;
    }

    if (!table->isColumn(parsedQuery.groupby_return_aggregate_col))
    {
        cout << "SEMANTIC ERROR: Return aggregate column not present in table" << endl;
        return false;
    }

    if (!(parsedQuery.OP == LESS_THAN || parsedQuery.OP == GREATER_THAN || parsedQuery.OP == LEQ || parsedQuery.OP == GEQ || parsedQuery.OP == EQUAL))
    {
        cout << "SEMANTIC ERROR: Invalid having operator" << endl;
        return false;
    }

    if (!(parsedQuery.havingAggregateFunction == MAX || parsedQuery.havingAggregateFunction == MIN || parsedQuery.havingAggregateFunction == CNT || parsedQuery.havingAggregateFunction == SUM || parsedQuery.havingAggregateFunction == AVG))
    {
        cout << "SEMANTIC ERROR: Invalid having aggregate function" << endl;
        return false;
    }

    if (!(parsedQuery.returnAggregateFunction == MAX || parsedQuery.returnAggregateFunction == MIN || parsedQuery.returnAggregateFunction == CNT || parsedQuery.returnAggregateFunction == SUM || parsedQuery.returnAggregateFunction == AVG))
    {
        cout << "SEMANTIC ERROR: Invalid return aggregate function" << endl;
        return false;
    }

    return true;
}

void rename_page(string pageName, string modifiedPageName) {
    logger.log("renamePage");
    // cout << "Renaming " << pageName << " to " << modifiedPageName << endl;
    pageName = "../data/temp/" + pageName;
    modifiedPageName = "../data/temp/" + modifiedPageName;
    if (rename(pageName.c_str(), modifiedPageName.c_str()) != 0) {
        perror("Error renaming file");
    }
}

void executeGROUPBY()
{
    logger.log("executeGROUPBY");

    Table *oldTable = tableCatalogue.getTable(parsedQuery.groupByTableName);
    vector<string> columnName;
    columnName.push_back(parsedQuery.groupby_table_name);
    vector<SortingStrategy> strtgy;
    strtgy.push_back(ASC);

    // Sort the tempTable according to the column name and sorting strategy provided
    oldTable->sort(columnName, strtgy);
    
    Table *tempTable = new Table("TEMP" + oldTable->tableName, oldTable->columns);
    tempTable->distinctValuesPerColumnCount = oldTable->distinctValuesPerColumnCount;
    tempTable->blockCount = oldTable->blockCount;
    tempTable->rowCount = oldTable->rowCount;
    tempTable->rowsPerBlockCount = oldTable->rowsPerBlockCount;

    for (int i = 0; i < oldTable->blockCount; i++) {
    // read block i from oldTable
        Page p = bufferManager.getPage(oldTable->tableName, i);
        // write to block i of tempTable
        p.pageName = "../data/temp/" + tempTable->tableName + "_Page" + to_string(i);
        p.writePage();
    }
    tempTable->blockCount = oldTable->blockCount;
    tempTable->rowCount   = oldTable->rowCount;
    tempTable->rowsPerBlockCount = oldTable->rowsPerBlockCount;
    tableCatalogue.insertTable(tempTable);

    // for (int i = 0; i < oldTable->blockCount; i++)
    // {
    //     string pgOldName = oldTable->tableName + "_Page" + to_string(i);
    //     string pgNewName = tempTable->tableName + "_Page" + to_string(i);
    //     rename_page(pgOldName, pgNewName);
    // }
    tempTable->sort(columnName, strtgy);
    // tableCatalogue.insertTable(tempTable);
  
    vector<string> columnNames;
    columnNames.push_back(parsedQuery.groupby_table_name);
    columnNames.push_back(parsedQuery.groupby_return_func_col);

    // map1 -> having aggregate   map2 -> return aggregate
    map<int, double> map1, map2, map_cnt;
    map<int, double>::iterator it;

    Cursor cursor = tempTable->getCursor();
    vector<int> row = cursor.getNext();
    while (row.size() != 0)
    {
        int grp_key = row[tempTable->getColumnIndex(parsedQuery.groupby_table_name)];
        map_cnt[grp_key] += 1;

        int aggregateValue = row[tempTable->getColumnIndex(parsedQuery.groupby_have_aggregate_col)];
        int returnAggregateValue = row[tempTable->getColumnIndex(parsedQuery.groupby_return_aggregate_col)];

        if (map1.find(grp_key) == map1.end())
        {
            // cout << aggregateValue << endl;
            // map1[grp_key] = aggregateValue;
            if (parsedQuery.havingAggregateFunction == CNT) {
                map1[grp_key] = 1;
            } 
            else {
        // e.g. for SUM, MIN, MAX, or AVG – store the first row’s column value
                map1[grp_key] = aggregateValue;
            }
        }
        else
        {
            if (parsedQuery.havingAggregateFunction == MAX)
            {
                map1[grp_key] = max(map1[grp_key], (double)aggregateValue);
            }
            else if (parsedQuery.havingAggregateFunction == MIN)
            {
                map1[grp_key] = min(map1[grp_key], (double)aggregateValue);
            }
            else if (parsedQuery.havingAggregateFunction == CNT)
            {
                map1[grp_key] += 1;
            }
            else if (parsedQuery.havingAggregateFunction == SUM)
            {
                map1[grp_key] += aggregateValue;
            }
            else if (parsedQuery.havingAggregateFunction == AVG)
            {
                map1[grp_key] += aggregateValue;
            }
        }

        if (map2.find(grp_key) == map2.end())
        {
            // map2[grp_key] = returnAggregateValue;
            if (parsedQuery.returnAggregateFunction == CNT) {
                map2[grp_key] = 1;
            } 
            else {
        // e.g. for SUM, MIN, MAX, or AVG – store the first row’s column value
                map2[grp_key] = returnAggregateValue;
            }
        }
        else
        {
            if (parsedQuery.returnAggregateFunction == MAX)
            {
                map2[grp_key] = max(map2[grp_key], (double)returnAggregateValue);
            }
            else if (parsedQuery.returnAggregateFunction == MIN)
            {
                map2[grp_key] = min(map2[grp_key], (double)returnAggregateValue);
            }
            else if (parsedQuery.returnAggregateFunction == CNT)
            {
                map2[grp_key] += 1;
            }
            else if (parsedQuery.returnAggregateFunction == SUM)
            {
                map2[grp_key] += returnAggregateValue;
            }
            else if (parsedQuery.returnAggregateFunction == AVG)
            {
                map2[grp_key] += returnAggregateValue;
            }
        }
        row = cursor.getNext();
    }
    // Delete the temporary files
    // for (int i = 0; i < tempTable->blockCount; i++)
    // {
    //     string pgName = "../data/temp/" + tempTable->tableName + "_Page" + to_string(i);
    //     remove(pgName.c_str());
    // }

    // Process each group key
    for (it = map1.begin(); it != map1.end(); ++it)
    {
        int grp_key = it->first;
        // Adjust average for having aggregate if needed
        if (parsedQuery.havingAggregateFunction == AVG)
        {
            map1[grp_key] = map1[grp_key] / (double)map_cnt[grp_key];
        }
        // Adjust average for return aggregate if needed
        if (parsedQuery.returnAggregateFunction == AVG)
        {
            map2[grp_key] = map2[grp_key] / (double)map_cnt[grp_key];
        }
    }
    
    string fileName = "../data/" + parsedQuery.groupby_res_table_name + ".csv";
    ofstream fout(fileName);
    fout << parsedQuery.groupby_table_name << "," << parsedQuery.groupby_return_func_col << endl;

    // Loop over map1
    for (it = map1.begin(); it != map1.end(); ++it)
    {
        int grp_key = it->first;
        double havingAggregateValue = it->second;
        int returnAggregateValue = map2[grp_key];

        if ((parsedQuery.OP == LESS_THAN && havingAggregateValue < (double)stoi(parsedQuery.groupby_val))
         || (parsedQuery.OP == GREATER_THAN && havingAggregateValue > (double)stoi(parsedQuery.groupby_val))
         || (parsedQuery.OP == LEQ && havingAggregateValue <= (double)stoi(parsedQuery.groupby_val))
         || (parsedQuery.OP == GEQ && havingAggregateValue >= (double)stoi(parsedQuery.groupby_val)))
        {
            fout << grp_key << "," << returnAggregateValue << endl;
        }
    }
    fout.close();

    // Remove temporary files
    // string tempFileName = "../data/temp/" + tempTable->tableName + ".csv";
    // remove(tempFileName.c_str());
    // tempFileName = "../data/temp/" + parsedQuery.groupby_res_table_name + ".csv";
    // remove(tempFileName.c_str());

    Table *table = new Table(parsedQuery.groupby_res_table_name);
    if (table->load())
    {
        tableCatalogue.insertTable(table);
        cout << "Loaded Table. Column Count: " << table->columnCount << " Row Count: " << table->rowCount << endl;
    }
    return;
}

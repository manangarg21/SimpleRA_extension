#include "global.h"
/**
 * @brief 
 * SYNTAX: SOURCE filename
 */
bool syntacticParseSOURCE()
{
    logger.log("syntacticParseSOURCE");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = SOURCE;
    parsedQuery.sourceFileName = tokenizedQuery[1];
    return true;
}

bool semanticParseSOURCE()
{
    logger.log("semanticParseSOURCE");
    if (!isQueryFile(parsedQuery.sourceFileName))
    {
        cout << "SEMANTIC ERROR: File doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeSOURCE()
{
    logger.log("executeSOURCE");
    string filePath = "../data/" + parsedQuery.sourceFileName + ".ra";

    ifstream fin(filePath, ios::in);
    if (!fin.is_open())
    {
        cout << "ERROR: Unable to open file: " << filePath << endl;
        return;
    }

    logger.log("Reading Commands from File: ");
    logger.log(parsedQuery.sourceFileName);

    string command; 
    while (getline(fin, command)){ //assumes '\n' to be delimeter for query
        tokenizedQuery.clear();
        parsedQuery.clear();

        logger.log("\nReading New Command: ");
        logger.log(command);

        //same code as for general query execution
        regex delim("[^\\s,]+");
        auto words_begin = std::sregex_iterator(command.begin(), command.end(), delim);
        auto words_end = std::sregex_iterator();
        for (std::sregex_iterator i = words_begin; i != words_end; ++i)
            tokenizedQuery.emplace_back((*i).str());

        if (tokenizedQuery.size() == 1 && tokenizedQuery.front() == "QUIT")
        {
            break;
        }

        if (tokenizedQuery.empty())
        {
            continue;
        }

        if (tokenizedQuery.size() == 1)
        {
            cout << "SYNTAX ERROR" << endl;
            continue;
        }

        //not executing doCommand directly- to be modified
        logger.log("doCommand");
        if (syntacticParse() && semanticParse())
            executeCommand();
    }

    return;
}

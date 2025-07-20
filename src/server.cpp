// Server Code
#include "global.h"

using namespace std;

float BLOCK_SIZE = 1;
uint BLOCK_COUNT = 2; //Q: Left as is, can we ask TA to change //TBD: assumption: changed for P2, P3
uint BLOCK_COUNT_P2 = 10; //Q: can this be temporarily changed
uint PRINT_COUNT = 20;
Logger logger;
vector<string> tokenizedQuery;
ParsedQuery parsedQuery;
TableCatalogue tableCatalogue;
MatrixCatalogue matrixCatalogue;
BufferManager bufferManager;

int NO_OF_BLOCK_READ;
int NO_OF_BLOCK_WRITTEN;
int NO_OF_BLOCK_ACCESSED;

void doCommand() {

    NO_OF_BLOCK_READ = 0;
    NO_OF_BLOCK_WRITTEN = 0;
    NO_OF_BLOCK_ACCESSED = 0;

    logger.log("doCommand");
    if (syntacticParse() && semanticParse())
        executeCommand();

    NO_OF_BLOCK_ACCESSED = NO_OF_BLOCK_READ + NO_OF_BLOCK_WRITTEN;
    if (NO_OF_BLOCK_ACCESSED) {
        // cout << "No. of Block Read = " << to_string(NO_OF_BLOCK_READ) << endl;
        // cout << "No. of Block Written = " << to_string(NO_OF_BLOCK_WRITTEN)
        //      << endl;
        // cout << "No. of Block Accessed = " << to_string(NO_OF_BLOCK_ACCESSED)
        //      << endl;
    }

    return;
}

int main(void) {

    regex delim("[^\\s,]+");
    string command;
    system("rm -rf ../data/temp");
    system("mkdir ../data/temp");

    while (!cin.eof()) {
        cout << "\n> ";
        tokenizedQuery.clear();
        parsedQuery.clear();
        logger.log("\nReading New Command: ");
        getline(cin, command);
        logger.log(command);

        auto words_begin =
            std::sregex_iterator(command.begin(), command.end(), delim);
        auto words_end = std::sregex_iterator();
        for (std::sregex_iterator i = words_begin; i != words_end; ++i)
            tokenizedQuery.emplace_back((*i).str());

        if (tokenizedQuery.size() == 1 && tokenizedQuery.front() == "QUIT") {
            break;
        }

        if (tokenizedQuery.empty()) {
            continue;
        }

        if (tokenizedQuery.size() == 1) {
            cout << "SYNTAX ERROR" << endl;
            continue;
        }

        doCommand();
    }
}
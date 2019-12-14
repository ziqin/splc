#include <cstdio>
#include <iostream>
#include <memory>
#include "parser.hpp"

using namespace std;

#define CMD_ERR     0x1
#define IO_ERR      0x2
#define PARSING_ERR 0x4


static string targetPathOf(const string& srcPath) {
    static const string suffix = ".spl";

    size_t prefixLen = srcPath.length() - suffix.length();
    if (prefixLen < 0 || srcPath.compare(prefixLen, suffix.length(), suffix) != 0) {
        throw invalid_argument("Invalid source file path!");
    }

    return srcPath.substr(0, prefixLen) + ".out";
}


int main(int argc, const char ** argv) {
    // check cli arguments
    if (argc != 2) {
        cerr << "Usage:\n\t" << argv[0] << " /path/to/source/file.spl" << endl;
        exit(CMD_ERR);
    }

    // filenames
    const char * srcPath = argv[1];
    string targetPath = targetPathOf(srcPath);

    FILE * srcFile;
    if (!(srcFile = fopen(srcPath, "r")) || !freopen(targetPath.c_str(), "w", stderr)) {
        cerr << "Failed to open file(s)" << endl;
        exit(IO_ERR);
    }

    // parsing
    unique_ptr<AST::Program> ast(parseFile(srcFile));
    fclose(srcFile);
    if (!ast) { // lexical/syntax error
        exit(PARSING_ERR);
    }

    // write parse tree to file
    FILE * targetFile = fopen(targetPath.c_str(), "w");
    if (!targetFile) {
        cerr << "Failed to open file " << targetPath << " to write" << endl;
        exit(IO_ERR);
    }
    // parseTree->fprint(targetFile, 0);
    fclose(targetFile);

    return 0;
}

#include <cstdio>
#include <cstring>
#include <memory>
#include "cst.hpp"
#include "parser.hpp"
#include "ast.hpp"

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
        fprintf(stderr, "Usage:\n\t%s /path/to/source/file.spl\n", argv[0]);
        exit(CMD_ERR);
    }

    // filenames
    const char * srcPath = argv[1];
    string targetPath = targetPathOf(srcPath);

    FILE * srcFile;
    if (!(srcFile = fopen(srcPath, "r")) || !freopen(targetPath.c_str(), "w", stderr)) {
        fprintf(stderr, "Failed to open file(s)\n");
        exit(IO_ERR);
    }

    // parsing
    unique_ptr<CST::Node> parseTree(parseFile(srcFile));
    fclose(srcFile);
    if (!parseTree) { // lexical/syntax error
        exit(PARSING_ERR);
    }

    // write parse tree to file
    FILE * targetFile = fopen(targetPath.c_str(), "w");
    if (!targetFile) {
        fprintf(stderr, "Failed to open file %s to write\n", targetPath.c_str());
        exit(IO_ERR);
    }
    parseTree->fprint(targetFile, 0);
    fclose(targetFile);

    // concrete syntax tree -> abstract syntax tree
    unique_ptr<AST::Program> program(new AST::Program(*parseTree));

    return 0;
}

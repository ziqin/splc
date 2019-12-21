#include <cstdio>
#include <iostream>
#include <memory>
#include "parser.hpp"
#include "ast_dump.hpp"
#include "semantic.hpp"

using namespace std;

const int CMD_ERR       = 0x1;
const int IO_ERR        = 0x2;
const int PARSING_ERR   = 0x4;


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
    if (!(srcFile = fopen(srcPath, "r"))) {
        cerr << "Failed to open file(s)" << endl;
        exit(IO_ERR);
    }

    // parsing
    unique_ptr<AST::Program> ast(parseFile(srcFile));
    fclose(srcFile);
    if (!ast) exit(PARSING_ERR);

    auto dumpWalker = make_unique<AST::DumpWalker>(std::cout);
    auto scopeSetter = make_unique<smt::ScopeSetter>();
    ast->traverse({ dumpWalker.get(), scopeSetter.get() }, nullptr);

    return 0;
}

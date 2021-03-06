#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
// #include "ast_dump.hpp"
#include "parser.hpp"
#include "semantic.hpp"
#include "gen_tac.hpp"

using namespace std;

const int CMD_ERR       = 0x1;
const int IO_ERR        = 0x2;
const int PARSING_ERR   = 0x4;
const int SEMANTIC_ERR  = 0x8;


static string targetPathOf(const string& srcPath) {
    static const string suffix = ".spl";

    size_t prefixLen = srcPath.length() - suffix.length();
    if (prefixLen < 0 || srcPath.compare(prefixLen, suffix.length(), suffix) != 0) {
        throw invalid_argument("Invalid source file path!");
    }

    return srcPath.substr(0, prefixLen) + ".ir";
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
    unique_ptr<ast::Program> ast(parseFile(srcFile));
    fclose(srcFile);
    if (!ast) exit(PARSING_ERR);

    // // dump ast
    // auto printer = make_unique<ast::Printer>(cout);
    // ast->traverse({ printer.get() });

    // semantic analysis
    auto semanticErrs = smt::analyzeSemantic(ast.get());
    if (!semanticErrs.empty()) {
        for (auto& semanticErr: semanticErrs) {
            cerr << semanticErr << std::endl;
        }
        exit(SEMANTIC_ERR);
    }

    // intermediate code generation
    auto tacGenerator = make_unique<ir::TacGenerator>(ast.get());
    ofstream fout(targetPath);
    for (auto tacPtr: tacGenerator->getTac()) {
        fout << *tacPtr << std::endl;
    }
    fout.close();

    return 0;
}

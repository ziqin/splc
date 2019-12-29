#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include "parser.hpp"
#include "semantic.hpp"
#include "translate.hpp"

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
    unique_ptr<AST::Program> ast(parseFile(srcFile));
    fclose(srcFile);
    if (!ast) exit(PARSING_ERR);

    // semantic analysis
    vector<smt::SemanticErrRecord> semanticErrs;
    auto scopeSetter = make_unique<smt::ScopeSetter>();
    auto structInit = make_unique<smt::StructInitializer>(semanticErrs); 
    auto symbolSetter = make_unique<smt::SymbolSetter>(semanticErrs);
    auto typeSynthesizer = make_unique<smt::TypeSynthesizer>(semanticErrs);
    ast->traverse({ scopeSetter.get(), structInit.get() }, nullptr);
    ast->traverse({ symbolSetter.get(), typeSynthesizer.get() }, nullptr);
    if (semanticErrs.size() > 0) {
        for (auto& err: semanticErrs) {
            cerr << "Error type " << err.err - smt::ERR_TYPE0 << " at Line " << err.cause->loc.end.line << ": " << err.msg << endl;
        }
        exit(SEMANTIC_ERR);
    }

    // intermediate code generation
    auto tacGenerator = make_unique<gen::TacGenerator>();
    ast->traverse({ tacGenerator.get() }, nullptr);
    ofstream fout(targetPath);
    tacGenerator->printTac(fout);

    return 0;
}

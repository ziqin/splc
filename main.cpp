#include <cstdio>
#include <cstring>
#include <memory>
#include "cst.hpp"
#include "parser.hpp"

using namespace std;

#define CMD_ERR     0x1
#define IO_ERR      0x2
#define PARSING_ERR 0x4


static char * getTargetPath(const char * srcPath) {
    int name_length = strlen(srcPath);
    int prefixLength = name_length - 4;
    if (prefixLength < 0 || strcmp(srcPath + prefixLength, ".spl") != 0) {  // not ends with .spl
        prefixLength = name_length;
    }
    char * targetPath = (char *) malloc(prefixLength + 5);
    memcpy(targetPath, srcPath, prefixLength);
    strcpy(targetPath + prefixLength, ".out");
    return targetPath;
}


int main(int argc, const char ** argv) {
    // check cli arguments
    if (argc != 2) {
        fprintf(stderr, "Usage:\n\t%s /path/to/source/file.spl\n", argv[0]);
        exit(CMD_ERR);
    }

    // filenames
    const char * srcPath = argv[1];
    char * targetPath = getTargetPath(srcPath);

    FILE * srcFile;
    if (!(srcFile = fopen(srcPath, "r")) || !freopen(targetPath, "w", stderr)) {
        fprintf(stderr, "Failed to open file(s)\n");
        exit(IO_ERR);
    }

    // parsing
    std::unique_ptr<CST::Node> parseTree(parseFile(srcFile));
    if (!parseTree) { // lexical/syntax error
        exit(PARSING_ERR);
    }

    // write parse tree to file
    FILE * targetFile = fopen(targetPath, "w");
    if (!targetFile) {
        fprintf(stderr, "Failed to open file %s to write\n", targetPath);
        exit(IO_ERR);
    }
    parseTree->fprint(targetFile, 0);
    fclose(targetFile);

    return 0;
}

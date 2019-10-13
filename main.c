#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, const char ** argv) {
    if (argc == 2) {
        const char * filename = argv[1];
        FILE * src_file = fopen(filename, "r");
        if (src_file == NULL) {
            fprintf(stderr, "Failed to open file %s\n", filename);
            exit(1);
        }
        AstNode * ast = build_ast(src_file);
        if (ast) {
            fprint_ast_node(stdout, ast, 0);
            delete_ast_node(ast);
        } else { // lexical/syntax error
            exit(2);
        }
    } else {
        fprintf(stderr, "Usage:\n\t%s /path/to/source/file.spl\n", argv[0]);
        exit(3);
    }
    return 0;
}

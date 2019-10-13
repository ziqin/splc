#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define IO_ERR      0x1
#define PARSING_ERR 0x2
#define CMD_ERR     0x4


int main(int argc, const char ** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage:\n\t%s /path/to/source/file.spl\n", argv[0]);
        exit(CMD_ERR);
    }

    // filenames
    const char * src_path = argv[1];
    int name_length = strlen(src_path);
    char * target_path = (char *)malloc(sizeof(char) * (name_length + 5));
    int prefix_length = name_length - 4;
    if (name_length >= 4 && strcmp(src_path + prefix_length, ".spl") == 0) {    // ends with .spl
        memcpy(target_path, src_path, sizeof(char) * prefix_length);
        strcpy(target_path + prefix_length, ".out");          // .spl -> .out
    } else {
        memcpy(target_path, src_path, sizeof(char) * name_length);
        strcpy(target_path + name_length, ".out");              // append .out
    }

    // redirect stderr to target_path
    freopen(target_path, "w", stderr);

    errno = 0;
    AstNode * ast = parse_file(src_path);
    if (!ast) // lexical/syntax error
        exit((errno & IO_ERR) ? IO_ERR : PARSING_ERR);

    int ret_code = 0;
    FILE * target_file = fopen(target_path, "w");
    if (!target_file) {
        fprintf(stderr, "Failed to open file %s to write, errno=%d\n", target_path, errno);
        ret_code = IO_ERR;
    }
    fprint_ast_node(target_file, ast, 0);
    fclose(target_file);
    delete_ast_node(ast);
    free(target_path);

    return ret_code;
}


AstNode * parse_file(const char * src_path) {
    AstNode * ast = NULL;
    FILE * src_file = fopen(src_path, "r");
    if (src_file) {
        ast = build_ast(src_file);
    } else {
        errno |= IO_ERR;
        fprintf(stderr, "Failed to open file %s, errno=%d\n", src_path, errno);
    }
    return ast;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cst.h"
#include "parser.h"

#define CMD_ERR     0x1
#define IO_ERR      0x2
#define PARSING_ERR 0x4


static char * get_target_path(const char * src_path) {
    int name_length = strlen(src_path);
    int prefix_length = name_length - 4;
    if (prefix_length < 0 || strcmp(src_path + prefix_length, ".spl") != 0) {  // not ends with .spl
        prefix_length = name_length;
    }
    char * target_path = (char *) malloc(prefix_length + 5);
    memcpy(target_path, src_path, prefix_length);
    strcpy(target_path + prefix_length, ".out");
    return target_path;
}


int main(int argc, const char ** argv) {
    // check cli arguments
    if (argc != 2) {
        fprintf(stderr, "Usage:\n\t%s /path/to/source/file.spl\n", argv[0]);
        exit(CMD_ERR);
    }

    // filenames
    const char * src_path = argv[1];
    char * target_path = get_target_path(src_path);

    FILE * src_file;
    if (!(src_file = fopen(src_path, "r")) || !freopen(target_path, "w", stderr)) {
        fprintf(stderr, "Failed to open file(s)\n");
        exit(IO_ERR);
    }

    // parsing
    cst_node_t * parse_tree = parse_file(src_file);
    if (!parse_tree) { // lexical/syntax error
        exit(PARSING_ERR);
    }

    // write parse tree to file
    FILE * target_file = fopen(target_path, "w");
    if (!target_file) {
        fprintf(stderr, "Failed to open file %s to write\n", target_path);
        exit(IO_ERR);
    }
    fprint_cst_node(target_file, parse_tree, 0);
    fclose(target_file);
    delete_cst_node(parse_tree);
    free(target_path);

    return 0;
}

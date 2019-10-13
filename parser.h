#ifndef PARSER_H
#define PARSER_H

// header file adapter
#include <stdio.h>
#include "ast.h"

ast_node_t * build_ast(FILE *);
ast_node_t * parse_file(const char * src_path);

#endif

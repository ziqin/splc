#ifndef PARSER_H
#define PARSER_H

// header file adapter
#include <stdio.h>
#include "ast.h"

AstNode * build_ast(FILE *);
AstNode * parse_file(const char * src_path);

#endif

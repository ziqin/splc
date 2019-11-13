#ifndef PARSER_H
#define PARSER_H

// header file adapter
#include <stdio.h>
#include "cst.h"

cst_node_t * build_cst(FILE *);
cst_node_t * parse_file(const char * src_path);

#endif

#ifndef PARSER_H
#define PARSER_H

// header file for libparser.a

struct FILE;
struct cst_node_t;

cst_node_t * parse_file(FILE *);

#endif

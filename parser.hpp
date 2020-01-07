#ifndef PARSER_H
#define PARSER_H

// header file for libparser.a

#include <cstdio>
#include "ast.hpp"

ast::Program * parseFile(FILE *);
ast::Program * parseStr(const char *);

#endif

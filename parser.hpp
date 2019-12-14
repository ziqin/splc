#ifndef PARSER_H
#define PARSER_H

// header file for libparser.a

#include <cstdio>
#include "ast.hpp"

AST::Program * parseFile(FILE *);

#endif

#ifndef AST_H
#define AST_H

#include <stdarg.h>
#include <stdio.h>

typedef enum {
    AST_INT,
    AST_FLOAT,
    AST_TYPE,
    AST_ID, 
    AST_CHAR,
    AST_STRUCT, 
    AST_IF, 
    AST_ELSE, 
    AST_WHILE, 
    AST_FOR,
    AST_RETURN, 
    AST_DOT, 
    AST_SEMI, 
    AST_COMMA,
    AST_ASSIGN, 
    AST_LT, 
    AST_LE, 
    AST_GT, 
    AST_GE, 
    AST_NE,
    AST_EQ, 
    AST_PLUS, 
    AST_MINUS, 
    AST_MUL,
    AST_DIV, 
    AST_AND, 
    AST_OR, 
    AST_NOT, 
    AST_LP, 
    AST_RP, 
    AST_LB, 
    AST_RB, 
    AST_LC, 
    AST_RC,
    AST_Program, 
    AST_ExtDefList, 
    AST_ExtDef,
    AST_ExtDecList, 
    AST_Specifier, 
    AST_StructSpecifier, 
    AST_VarDec,
    AST_FunDec,
    AST_VarList,
    AST_ParamDec,
    AST_CompSt,
    AST_StmtList,
    AST_Stmt,
    AST_DefList,
    AST_Def,
    AST_DecList,
    AST_Dec,
    AST_Exp,
    AST_Args,
} ast_type_name;

typedef struct ast_node_t {
    ast_type_name node_type;
} ast_node_t;

typedef struct {
    ast_node_t proto;
    int value;
} int_ast_node_t;

typedef struct {
    ast_node_t proto;
    float value;
} float_ast_node_t;

typedef struct {
    ast_node_t proto;
    char * value;
} str_ast_node_t;

// nonterminal
typedef struct {
    ast_node_t proto;
    int first_line;
    int length;
} nt_ast_node_t;

struct YYLTYPE;

ast_node_t * create_ast_node(ast_type_name);
ast_node_t * create_int_ast_node(ast_type_name, int);
ast_node_t * create_float_ast_node(ast_type_name, float);
ast_node_t * create_str_ast_node(ast_type_name, const char *);
ast_node_t * create_nt_ast_node(ast_type_name, const struct YYLTYPE *, int, ...);
void delete_ast_node(ast_node_t *);
void fprint_ast_node(FILE * fp, const ast_node_t * node, int indent);

#endif

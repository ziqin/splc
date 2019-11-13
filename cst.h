#ifndef CST_H
#define CST_H

#include <stdarg.h>
#include <stdio.h>

typedef enum {
    CST_INT,
    CST_FLOAT,
    CST_TYPE,
    CST_ID, 
    CST_CHAR,
    CST_STRUCT, 
    CST_IF, 
    CST_ELSE, 
    CST_WHILE, 
    CST_FOR,
    CST_RETURN, 
    CST_DOT, 
    CST_SEMI, 
    CST_COMMA,
    CST_ASSIGN, 
    CST_LT, 
    CST_LE, 
    CST_GT, 
    CST_GE, 
    CST_NE,
    CST_EQ, 
    CST_PLUS, 
    CST_MINUS, 
    CST_MUL,
    CST_DIV, 
    CST_AND, 
    CST_OR, 
    CST_NOT, 
    CST_LP, 
    CST_RP, 
    CST_LB, 
    CST_RB, 
    CST_LC, 
    CST_RC,
    CST_Program, 
    CST_ExtDefList, 
    CST_ExtDef,
    CST_ExtDecList, 
    CST_Specifier, 
    CST_StructSpecifier, 
    CST_VarDec,
    CST_FunDec,
    CST_VarList,
    CST_ParamDec,
    CST_CompSt,
    CST_StmtList,
    CST_Stmt,
    CST_DefList,
    CST_Def,
    CST_DecList,
    CST_Dec,
    CST_Exp,
    CST_Args,
} cst_type_name;

typedef struct cst_node_t {
    cst_type_name node_type;
} cst_node_t;

typedef struct {
    cst_node_t proto;
    int value;
} int_cst_node_t;

typedef struct {
    cst_node_t proto;
    float value;
} float_cst_node_t;

typedef struct {
    cst_node_t proto;
    char * value;
} str_cst_node_t;

// nonterminal
typedef struct {
    cst_node_t proto;
    int first_line;
    int length;
} nt_cst_node_t;

struct YYLTYPE;

cst_node_t * create_cst_node(cst_type_name);
cst_node_t * create_int_cst_node(cst_type_name, int);
cst_node_t * create_float_cst_node(cst_type_name, float);
cst_node_t * create_str_cst_node(cst_type_name, const char *);
cst_node_t * create_nt_cst_node(cst_type_name, const struct YYLTYPE *, int, ...);
void delete_cst_node(cst_node_t *);
void fprint_cst_node(FILE * fp, const cst_node_t * node, int indent);

#endif

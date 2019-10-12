#ifndef AST_H
#define AST_H

#include <stdarg.h>

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
} AstNodeType;

typedef struct AstNode {
    AstNodeType node_type;
} AstNode;

typedef struct {
    AstNode proto;
    int value;
} IntAstNode;

typedef struct {
    AstNode proto;
    float value;
} FloatAstNode;

typedef struct {
    AstNode proto;
    const char * value;
} StringAstNode;

typedef struct {
    AstNode proto;
    int length;
    AstNode ** children;
} ArrayAstNode;

AstNode * create_ast_node(AstNodeType);
AstNode * create_int_ast_node(AstNodeType, int);
AstNode * create_float_ast_node(AstNodeType, float);
AstNode * create_str_ast_node(AstNodeType, const char *);
AstNode * create_array_ast_node(AstNodeType, int, ...);
void delete_ast_node(AstNode *);
const char * ast_type_to_name(AstNodeType);

#endif
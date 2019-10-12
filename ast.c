#include <stdlib.h>
#include <string.h>
#include "ast.h"


AstNode * create_ast_node(AstNodeType type) {
    AstNode * node = (AstNode*)malloc(sizeof(AstNode));
    node->node_type = type;
    return node;
}


AstNode * create_int_ast_node(AstNodeType type, int val) {
    IntAstNode * node = (IntAstNode*)malloc(sizeof(IntAstNode));
    node->proto.node_type = type;
    node->value = val;
    return (AstNode*)node;
}


AstNode * create_float_ast_node(AstNodeType type, float val) {
    FloatAstNode * node = (FloatAstNode*)malloc(sizeof(FloatAstNode));
    node->proto.node_type = type;
    node->value = val;
    return (AstNode*)node;
}


AstNode * create_str_ast_node(AstNodeType type, const char * val) {
    StringAstNode * node = (StringAstNode*)malloc(sizeof(AstNode));
    node->proto.node_type = type;
    node->value = strdup(val);  // need to free
    return (AstNode*)node;
}


AstNode * create_array_ast_node(AstNodeType type, int count, ...) {
    ArrayAstNode * node = (ArrayAstNode*)malloc(sizeof(ArrayAstNode) + count*sizeof(AstNode*));
    node->proto.node_type = type;
    node->length = count;

    node->children = (AstNode**)(node + 1);
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i)
        node->children[i] = va_arg(args, AstNode*);
    va_end(args);
    return (AstNode*)node;
}


void delete_ast_node(AstNode * node) {
    switch (node->node_type) {
    case AST_CHAR:
    case AST_TYPE:
    case AST_ID:
        free((StringAstNode*)node);
        break;
    case AST_Program: 
    case AST_ExtDefList:
    case AST_ExtDef:
    case AST_ExtDecList: 
    case AST_Specifier:
    case AST_StructSpecifier:
    case AST_VarDec:
    case AST_FunDec:
    case AST_VarList:
    case AST_ParamDec:
    case AST_CompSt:
    case AST_StmtList:
    case AST_Stmt:
    case AST_DefList:
    case AST_Def:
    case AST_DecList:
    case AST_Dec:
    case AST_Exp:
    case AST_Args: {
        ArrayAstNode * array_node = (ArrayAstNode*)node;
        for (int i = 0; i < array_node->length; ++i)
            delete_ast_node(array_node->children[i]);
        free(node);
    } break;
    default:
        free(node);
    }
}


const char * ast_type_to_name(AstNodeType type) {
    static const char * ast_node_type_names[] = {
        "INT",      "FLOAT",    "TYPE",     "ID",       "CHAR",     "STRUCT",   "IF",       "ELSE", 
        "WHILE",    "RETURN",   "DOT",      "SEMI",     "COMMA",    "ASSIGN",   "LT",       "LE", 
        "GT",       "GE",       "NE",       "EQ",       "PLUS",     "MINUS",    "MUL",      "DIV", 
        "AND",      "OR",       "NOT",      "LP",       "RP",       "LB",       "RB",       "LC",
        "RC",
        "Program",      "ExtDefList",       "ExtDef",       "ExtDecList",
        "Specifier",    "StructSpecifier",  "VarDec",       "FunDec",
        "VarList",      "ParamDec",         "CompSt",       "StmtList", 
        "Stmt",         "DefList",          "Def",          "DecList", 
        "Dec",          "Exp",              "Args",
    };
    return ast_node_type_names[type];
}

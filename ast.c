#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "syntax.tab.h"


ast_node_t * create_ast_node(ast_type_name type) {
    ast_node_t * node = (ast_node_t*)malloc(sizeof(ast_node_t));
    node->node_type = type;
    return node;
}


ast_node_t * create_int_ast_node(ast_type_name type, int val) {
    int_ast_node_t * node = (int_ast_node_t*)malloc(sizeof(int_ast_node_t));
    node->proto.node_type = type;
    node->value = val;
    return (ast_node_t*)node;
}


ast_node_t * create_float_ast_node(ast_type_name type, float val) {
    float_ast_node_t * node = (float_ast_node_t*)malloc(sizeof(float_ast_node_t));
    node->proto.node_type = type;
    node->value = val;
    return (ast_node_t*)node;
}


ast_node_t * create_str_ast_node(ast_type_name type, const char * val) {
    str_ast_node_t * node = (str_ast_node_t*)malloc(sizeof(ast_node_t));
    node->proto.node_type = type;
    node->value = strdup(val);  // need to free
    return (ast_node_t*)node;
}


ast_node_t * create_nt_ast_node(ast_type_name type, const struct YYLTYPE * loc, int count, ...) {
    nt_ast_node_t * node = (nt_ast_node_t *)malloc(sizeof(nt_ast_node_t) + count * sizeof(ast_node_t *));
    node->proto.node_type = type;
    node->first_line = loc->first_line;
    node->length = count;
    ast_node_t ** children = (ast_node_t **)(node + 1);
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i)
        children[i] = va_arg(args, ast_node_t *);
    va_end(args);
    return (ast_node_t *)node;
}


void delete_ast_node(ast_node_t * node) {
    if (node == NULL) return;

    switch (node->node_type) {
    case AST_CHAR:
    case AST_TYPE:
    case AST_ID:
        free(((str_ast_node_t *)node)->value);
        free(node);
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
        nt_ast_node_t * nt_node = (nt_ast_node_t *)node;
        ast_node_t ** children = (ast_node_t **)(nt_node + 1);
        for (int i = 0; i < nt_node->length; ++i)
            delete_ast_node(children[i]);
        free(node);
    } break;
    default:
        free(node);
    }
}


static const char * ast_type2name[] = {
    "INT",      "FLOAT",    "TYPE",     "ID",       "CHAR",     "STRUCT",   "IF",       "ELSE", 
    "WHILE",    "FOR",      "RETURN",   "DOT",      "SEMI",     "COMMA",    "ASSIGN",   "LT",
    "LE",       "GT",       "GE",       "NE",       "EQ",       "PLUS",     "MINUS",    "MUL",
    "DIV",      "AND",      "OR",       "NOT",      "LP",       "RP",       "LB",       "RB",
    "LC",       "RC",
    "Program",      "ExtDefList",       "ExtDef",       "ExtDecList",
    "Specifier",    "StructSpecifier",  "VarDec",       "FunDec",
    "VarList",      "ParamDec",         "CompSt",       "StmtList", 
    "Stmt",         "DefList",          "Def",          "DecList", 
    "Dec",          "Exp",              "Args",
};


void fprint_ast_node(FILE * fp, const ast_node_t * node, int indent) {
    if (node == NULL) return;

    switch (node->node_type) {
    case AST_INT:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        fprintf(fp, "%s: %d\n", ast_type2name[node->node_type], ((int_ast_node_t *)node)->value);
        break;
    case AST_FLOAT:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        float val = ((float_ast_node_t *)node)->value;
        const char * fmt = val >= 1e-4 ? "%s: %f\n" : "%s: %E\n";
        fprintf(fp, fmt, ast_type2name[node->node_type], val);
        break;
    case AST_TYPE:
    case AST_ID:
    case AST_CHAR:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        fprintf(fp, "%s: %s\n", ast_type2name[node->node_type], ((str_ast_node_t *)node)->value);
        break;
    case AST_STRUCT: 
    case AST_IF:
    case AST_ELSE: 
    case AST_WHILE: 
    case AST_FOR:
    case AST_RETURN: 
    case AST_DOT:
    case AST_SEMI: 
    case AST_COMMA:
    case AST_ASSIGN: 
    case AST_LT:
    case AST_LE: 
    case AST_GT: 
    case AST_GE: 
    case AST_NE:
    case AST_EQ: 
    case AST_PLUS: 
    case AST_MINUS: 
    case AST_MUL:
    case AST_DIV: 
    case AST_AND: 
    case AST_OR: 
    case AST_NOT: 
    case AST_LP: 
    case AST_RP: 
    case AST_LB: 
    case AST_RB: 
    case AST_LC: 
    case AST_RC:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        fprintf(fp, "%s\n", ast_type2name[node->node_type]);
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
        nt_ast_node_t * nt_node = (nt_ast_node_t *)node;
        if (nt_node->length > 0) {
            for (int i = 0; i < indent; ++i) fputc(' ', fp);
            fprintf(fp, "%s (%d)\n", ast_type2name[nt_node->proto.node_type], nt_node->first_line);
            ast_node_t ** children = (ast_node_t **)(nt_node + 1);
            for (int i = 0; i < nt_node->length; ++i)
                fprint_ast_node(fp, children[i], indent+2);
        }
    } break;
    default: break;
    }
}

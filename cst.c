#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "cst.h"
#include "syntax.tab.h"
#include "utils/new.h"


cst_node_t * create_cst_node(cst_type_name type) {
    cst_node_t * node = NEW(cst_node_t);
    node->node_type = type;
    return node;
}


cst_node_t * create_int_cst_node(cst_type_name type, int val) {
    int_cst_node_t * node = NEW(int_cst_node_t);
    node->base.node_type = type;
    node->value = val;
    return (cst_node_t*)node;
}


cst_node_t * create_float_cst_node(cst_type_name type, float val) {
    float_cst_node_t * node = NEW(float_cst_node_t);
    node->base.node_type = type;
    node->value = val;
    return (cst_node_t*)node;
}


cst_node_t * create_str_cst_node(cst_type_name type, const char * val) {
    str_cst_node_t * node = NEW(str_cst_node_t);
    node->base.node_type = type;
    node->value = strdup(val);  // need to free
    return (cst_node_t*)node;
}


cst_node_t * create_nt_cst_node(cst_type_name type, const struct YYLTYPE * loc, int count, ...) {
    nt_cst_node_t * node = (nt_cst_node_t *)malloc(sizeof(nt_cst_node_t) + count * sizeof(cst_node_t *));
    node->base.node_type = type;
    node->first_line = loc->first_line;
    node->length = count;
    cst_node_t ** children = (cst_node_t **)(node + 1);
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i)
        children[i] = va_arg(args, cst_node_t *);
    va_end(args);
    return (cst_node_t *)node;
}


void delete_cst_node(cst_node_t * node) {
    if (node == NULL) return;

    switch (node->node_type) {
    case CST_CHAR:
    case CST_TYPE:
    case CST_ID:
        free(((str_cst_node_t *)node)->value);
        free(node);
        break;
    case CST_Program: 
    case CST_ExtDefList:
    case CST_ExtDef:
    case CST_ExtDecList: 
    case CST_Specifier:
    case CST_StructSpecifier:
    case CST_VarDec:
    case CST_FunDec:
    case CST_VarList:
    case CST_ParamDec:
    case CST_CompSt:
    case CST_StmtList:
    case CST_Stmt:
    case CST_DefList:
    case CST_Def:
    case CST_DecList:
    case CST_Dec:
    case CST_Exp:
    case CST_Args: {
        nt_cst_node_t * nt_node = (nt_cst_node_t *)node;
        cst_node_t ** children = (cst_node_t **)(nt_node + 1);
        for (int i = 0; i < nt_node->length; ++i)
            delete_cst_node(children[i]);
        free(node);
    } break;
    default:
        free(node);
    }
}


static const char * cst_type2name[] = {
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


void fprint_cst_node(FILE * fp, const cst_node_t * node, int indent) {
    if (node == NULL) return;

    switch (node->node_type) {
    case CST_INT:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        fprintf(fp, "%s: %d\n", cst_type2name[node->node_type], ((int_cst_node_t *)node)->value);
        break;
    case CST_FLOAT:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        float val = ((float_cst_node_t *)node)->value;
        const char * fmt = val >= 1e-4 ? "%s: %f\n" : "%s: %E\n";
        fprintf(fp, fmt, cst_type2name[node->node_type], val);
        break;
    case CST_TYPE:
    case CST_ID:
    case CST_CHAR:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        fprintf(fp, "%s: %s\n", cst_type2name[node->node_type], ((str_cst_node_t *)node)->value);
        break;
    case CST_STRUCT: 
    case CST_IF:
    case CST_ELSE: 
    case CST_WHILE: 
    case CST_FOR:
    case CST_RETURN: 
    case CST_DOT:
    case CST_SEMI: 
    case CST_COMMA:
    case CST_ASSIGN: 
    case CST_LT:
    case CST_LE: 
    case CST_GT: 
    case CST_GE: 
    case CST_NE:
    case CST_EQ: 
    case CST_PLUS: 
    case CST_MINUS: 
    case CST_MUL:
    case CST_DIV: 
    case CST_AND: 
    case CST_OR: 
    case CST_NOT: 
    case CST_LP: 
    case CST_RP: 
    case CST_LB: 
    case CST_RB: 
    case CST_LC: 
    case CST_RC:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        fprintf(fp, "%s\n", cst_type2name[node->node_type]);
        break;
    case CST_Program: 
    case CST_ExtDefList: 
    case CST_ExtDef:
    case CST_ExtDecList: 
    case CST_Specifier: 
    case CST_StructSpecifier: 
    case CST_VarDec:
    case CST_FunDec:
    case CST_VarList:
    case CST_ParamDec:
    case CST_CompSt:
    case CST_StmtList:
    case CST_Stmt:
    case CST_DefList:
    case CST_Def:
    case CST_DecList:
    case CST_Dec:
    case CST_Exp:
    case CST_Args: {
        nt_cst_node_t * nt_node = (nt_cst_node_t *)node;
        if (nt_node->length > 0) {
            for (int i = 0; i < indent; ++i) fputc(' ', fp);
            fprintf(fp, "%s (%d)\n", cst_type2name[nt_node->base.node_type], nt_node->first_line);
            cst_node_t ** children = (cst_node_t **)(nt_node + 1);
            for (int i = 0; i < nt_node->length; ++i)
                fprint_cst_node(fp, children[i], indent+2);
        }
    } break;
    default: break;
    }
}

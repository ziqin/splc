#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "syntax.tab.h"


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


AstNode * create_nt_ast_node(AstNodeType type, const struct YYLTYPE * loc, int count, ...) {
    NonterminallAstNode * node = (NonterminallAstNode*)malloc(sizeof(NonterminallAstNode) + count*sizeof(AstNode*));
    node->proto.node_type = type;
    node->first_line = loc->first_line;
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
    if (node == NULL) return;
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
        NonterminallAstNode * nt_node = (NonterminallAstNode*)node;
        for (int i = 0; i < nt_node->length; ++i)
            delete_ast_node(nt_node->children[i]);
        free(node);
    } break;
    default:
        free(node);
    }
}


static const char * ast_type2name[] = {
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


void fprint_ast_node(FILE * fp, const AstNode * node, int indent) {
    switch (node->node_type) {
    case AST_INT:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        fprintf(fp, "%s: %d\n", ast_type2name[node->node_type], ((IntAstNode*)node)->value);
        break;
    case AST_FLOAT:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        fprintf(fp, "%s: %f\n", ast_type2name[node->node_type], ((FloatAstNode*)node)->value);
        break;
    case AST_TYPE:
    case AST_ID:
    case AST_CHAR:
        for (int i = 0; i < indent; ++i) fputc(' ', fp);
        fprintf(fp, "%s: %s\n", ast_type2name[node->node_type], ((StringAstNode*)node)->value);
        break;
    case AST_STRUCT: 
    case AST_IF:
    case AST_ELSE: 
    case AST_WHILE: 
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
        NonterminallAstNode * nt_node = (NonterminallAstNode*)node;
        if (nt_node->length > 0) {
            for (int i = 0; i < indent; ++i) fputc(' ', fp);
            fprintf(fp, "%s (%d)\n", ast_type2name[nt_node->proto.node_type], nt_node->first_line);
            for (int i = 0; i < nt_node->length; ++i)
                fprint_ast_node(fp, nt_node->children[i], indent+2);
        }
    } break;
    default: break;
    }
}

#ifndef AST_H
#define AST_H

#include "cst.h"
#include "type.h"
#include "utils/array.h"


/* Expressions */

typedef struct ast_exp_t {
    enum {
        AST_LiteralExp,
        AST_IdentifierExp,
        AST_ArrayExp,
        AST_MemberExp,
        AST_UnaryExp,
        AST_BinaryExp,
        AST_AssignExp,
        AST_CallExp
    } node_type;
    type_t * type;
    // TODO: source code location
} ast_exp_t;

typedef struct ast_literal_exp_t {
    ast_exp_t base;
    union {
        char char_val;
        int int_val;
        float float_val;
    };
} ast_literal_exp_t;

typedef struct ast_id_exp_t {
    ast_exp_t base;
    char * identifier;
} ast_id_exp_t;

typedef struct ast_array_exp_t {
    ast_exp_t base;
    ast_exp_t * subject;
    ast_exp_t * index;
} ast_array_exp_t;

typedef struct ast_member_exp_t {
    ast_exp_t base;
    ast_exp_t * subject;
    ast_id_exp_t * member;
} ast_member_exp_t;

enum ast_opt_t {
    AST_OPT_AND,
    AST_OPT_OR,
    AST_OPT_LT,
    AST_OPT_LE,
    AST_OPT_GT,
    AST_OPT_GE,
    AST_OPT_NE,
    AST_OPT_EQ,
    AST_OPT_PLUS,
    AST_OPT_MINUS,
    AST_OPT_MUL,
    AST_OPT_DIV,
    AST_OPT_NOT   
};

typedef struct ast_unary_exp_t {
    ast_exp_t base;
    enum ast_opt_t opt;
    ast_exp_t * argument;
} ast_unary_exp_t;

typedef struct ast_binary_exp_t {
    ast_exp_t base;
    enum ast_opt_t opt;
    ast_exp_t * left, * right;
} ast_binary_exp_t;

typedef struct ast_assign_exp_t {
    ast_exp_t base;
    ast_exp_t * left, * right;
} ast_assign_exp_t;

typedef ast_exp_t * ast_exp_t_ptr;
USE_ARRAY(ast_exp_t_ptr);
typedef struct ast_call_exp_t {
    ast_exp_t base;
    ast_exp_t * callee;
    array_t(ast_exp_t_ptr) arguments;
} ast_call_exp_t;

ast_exp_t * create_ast_exp(cst_node_t * cst_node);
ast_exp_t * create_ast_literal_exp(cst_node_t * cst_node);
ast_exp_t * create_ast_id_exp(str_cst_node_t * cst_node);
ast_exp_t * create_ast_array_exp(nt_cst_node_t * cst_node);
ast_exp_t * create_ast_member_exp(nt_cst_node_t * cst_node);
ast_exp_t * create_ast_unary_exp(nt_cst_node_t * cst_node);
ast_exp_t * create_ast_binary_exp(nt_cst_node_t * cst_node);
ast_exp_t * create_ast_assign_exp(nt_cst_node_t * cst_node);
ast_exp_t * create_ast_call_exp(nt_cst_node_t * cst_node);

// TODO: destructors


/* Statements */

typedef struct ast_stmt_t {
    enum {
        AST_ExpStmt,
        AST_ReturnStmt,
        AST_IfStmt,
        AST_WhileStmt,
        AST_ForStmt,
        AST_CompStmt
    } node_type;
    // TODO: scope
} ast_stmt_t;

typedef struct ast_exp_stmt_t {
    ast_stmt_t base;
    ast_exp_t * expression;
} ast_exp_stmt_t;

typedef struct ast_return_stmt_t {
    ast_stmt_t base;
    ast_exp_t * argument;
} ast_return_stmt_t;

typedef struct ast_if_stmt_t {
    ast_stmt_t base;
    ast_exp_t * test;
    ast_stmt_t * consequent;
    ast_stmt_t * alternate;
} ast_if_stmt_t;

typedef struct ast_while_stmt_t {
    ast_stmt_t base;
    ast_exp_t * test;
    ast_stmt_t * body;
} ast_while_stmt_t;

typedef struct ast_for_stmt_t {
    ast_stmt_t base;
    ast_exp_t * init, * test, * update;
    ast_stmt_t * body;
} ast_for_stmt_t;

struct ast_var_def_t;
typedef struct ast_var_def_t * ast_var_def_t_ptr;
USE_ARRAY(ast_var_def_t_ptr);
typedef ast_stmt_t * ast_stmt_t_ptr;
USE_ARRAY(ast_stmt_t_ptr);
typedef struct ast_comp_stmt_t {
    ast_stmt_t base;
    array_t(ast_var_def_t_ptr) definitions;
    array_t(ast_stmt_t_ptr) body;
} ast_comp_stmt_t;

ast_stmt_t * create_ast_stmt(nt_cst_node_t * node);
ast_stmt_t * create_ast_exp_stmt(nt_cst_node_t * node);
ast_stmt_t * create_ast_return_stmt(nt_cst_node_t * node);
ast_stmt_t * create_ast_if_stmt(nt_cst_node_t * node);
ast_stmt_t * create_ast_while_stmt(nt_cst_node_t * node);
ast_stmt_t * create_ast_for_stmt(nt_cst_node_t * node);
ast_stmt_t * create_ast_comp_stmt(nt_cst_node_t * node);


/* Definitions */

typedef struct ast_def_t {
    enum {
        AST_VariableDef,
        AST_StructureDef,
        AST_FunctionDef
    } node_type;
    char * identifier;
    type_t * type;
} ast_def_t;

typedef struct ast_var_def_t {
    ast_def_t base;
    ast_exp_t * init;
} ast_var_def_t;

typedef struct ast_struct_def_t {
    ast_def_t base;
} ast_struct_def_t;

typedef struct ast_function_def_t {
    ast_def_t base;
    array_t(ast_var_def_t_ptr) parameters;
    ast_comp_stmt_t * statements; 
    // TODO: scope
} ast_function_def_t;


/* Program */

typedef ast_def_t * ast_def_t_ptr;
USE_ARRAY(ast_def_t_ptr);
typedef struct ast_program_t {
    array_t(ast_def_t_ptr) definitions;
    struct symbol_table_t * scope;
} ast_program_t;

#endif

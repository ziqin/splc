#include <string.h>
#include "ast.h"
#include "utils/new.h"


/* Expressions */

ast_exp_t * create_ast_literal_exp(cst_node_t * cst_node) {
    assert(cst_node->node_type == CST_CHAR  ||
           cst_node->node_type == CST_INT   ||
           cst_node->node_type == CST_FLOAT);
    ast_literal_exp_t * exp = NEW(ast_literal_exp_t);
    exp->base.node_type = AST_LiteralExp;
    switch (cst_node->node_type) {
    case CST_CHAR:
        exp->base.type = create_primitive_type(PRIMITIVE_CHAR);
        exp->char_val = ((float_cst_node_t *) cst_node)->value;
        break;
    case CST_INT:
        exp->base.type = create_primitive_type(PRIMITIVE_INT);
        exp->int_val = ((int_cst_node_t *) cst_node)->value;
        break;
    case CST_FLOAT:
        exp->base.type = create_primitive_type(PRIMITIVE_FLOAT);
        exp->float_val = ((float_cst_node_t *) cst_node)->value;
        break;
    default:
        break;
    }
    return (ast_exp_t *) exp;
}

ast_exp_t * create_ast_id_exp(str_cst_node_t * cst_node) {
    assert(cst_node->base.node_type == CST_ID);
    ast_id_exp_t * exp = NEW(ast_id_exp_t);
    exp->base.node_type = AST_IdentifierExp;
    exp->base.type = NULL;
    exp->identifier = strdup(cst_node->value);
    return (ast_exp_t *) exp;
}

ast_exp_t * create_ast_array_exp(nt_cst_node_t * cst_node) {
    cst_node_t ** children = (cst_node_t **)(cst_node + 1);
    assert(cst_node->base.node_type == CST_Exp && cst_node->length == 4);
    assert(children[0]->node_type == CST_Exp && children[1]->node_type == CST_LB &&
           children[2]->node_type == CST_Exp && children[3]->node_type == CST_RB);
    ast_array_exp_t * exp = NEW(ast_array_exp_t);
    exp->base.node_type = AST_ArrayExp;
    exp->base.type = NULL;
    exp->subject = create_ast_exp(children[0]);
    exp->index = create_ast_exp(children[2]);
    return (ast_exp_t *) exp;
}

ast_exp_t * create_ast_member_exp(nt_cst_node_t * cst_node) {
    cst_node_t ** children = (cst_node_t **)(cst_node + 1);
    assert(cst_node->base.node_type == CST_Exp && cst_node->length == 3);
    assert(children[0]->node_type == CST_Exp &&
           children[1]->node_type == CST_DOT &&
           children[2]->node_type == CST_ID);
    ast_member_exp_t * exp = NEW(ast_member_exp_t);
    exp->base.node_type = AST_MemberExp;
    exp->base.type = NULL;
    exp->subject = create_ast_exp(children[0]);
    exp->member = (ast_id_exp_t *) create_ast_id_exp((str_cst_node_t *) children[2]);
    return (ast_exp_t *) exp;
}

ast_exp_t * create_ast_unary_exp(nt_cst_node_t * cst_node) {
    cst_node_t ** children = (cst_node_t **)(cst_node + 1);
    assert(cst_node->base.node_type == CST_Exp && cst_node->length == 2);
    assert((children[0]->node_type == CST_MINUS || children[0]->node_type == CST_NOT) &&
           children[1]->node_type == CST_Exp);
    ast_unary_exp_t * exp = NEW(ast_unary_exp_t);
    exp->base.node_type = AST_UnaryExp;
    exp->base.type = NULL;
    exp->opt = children[0]->node_type == CST_MINUS ? AST_OPT_MINUS : AST_OPT_NOT;
    exp->argument = create_ast_exp(children[1]);
    return (ast_exp_t *) exp;
}

ast_exp_t * create_ast_binary_exp(nt_cst_node_t * cst_node) {
    cst_node_t ** children = (cst_node_t **)(cst_node + 1);
    assert(cst_node->base.node_type == CST_Exp  &&
           cst_node->length == 3                &&
           children[0]->node_type == CST_Exp    &&
           children[2]->node_type == CST_Exp    &&
           (children[1]->node_type == CST_AND   ||
            children[1]->node_type == CST_OR    ||
            children[1]->node_type == CST_LT    ||
            children[1]->node_type == CST_LE    ||
            children[1]->node_type == CST_GT    ||
            children[1]->node_type == CST_GE    ||
            children[1]->node_type == CST_NE    ||
            children[1]->node_type == CST_EQ    ||
            children[1]->node_type == CST_PLUS  ||
            children[1]->node_type == CST_MINUS ||
            children[1]->node_type == CST_MUL   ||
            children[1]->node_type == CST_DIV));
    ast_binary_exp_t * exp = NEW(ast_binary_exp_t);
    exp->base.node_type = AST_BinaryExp;
    exp->base.type = NULL;
    exp->left = create_ast_exp(children[0]);
    exp->right = create_ast_exp(children[2]);
    switch (children[1]->node_type) {
        case CST_AND:   exp->opt = AST_OPT_AND;     break;
        case CST_OR:    exp->opt = AST_OPT_OR;      break;
        case CST_LT:    exp->opt = AST_OPT_LT;      break;
        case CST_LE:    exp->opt = AST_OPT_LE;      break;
        case CST_GT:    exp->opt = AST_OPT_GT;      break;
        case CST_GE:    exp->opt = AST_OPT_GE;      break;
        case CST_NE:    exp->opt = AST_OPT_NE;      break;
        case CST_EQ:    exp->opt = AST_OPT_EQ;      break;
        case CST_PLUS:  exp->opt = AST_OPT_PLUS;    break;
        case CST_MINUS: exp->opt = AST_OPT_MINUS;   break;
        case CST_MUL:   exp->opt = AST_OPT_MUL;     break;
        case CST_DIV:   exp->opt = AST_OPT_DIV;     break;
        default:        break;
    }
    return (ast_exp_t *) exp;
}

ast_exp_t * create_ast_assign_exp(nt_cst_node_t * cst_node) {
    cst_node_t ** children = (cst_node_t **)(cst_node + 1);
    assert(cst_node->base.node_type == CST_Exp  &&
           cst_node->length == 3                &&
           children[0]->node_type == CST_Exp    &&
           children[1]->node_type == CST_ASSIGN &&
           children[2]->node_type == CST_Exp);
    ast_assign_exp_t * exp = NEW(ast_assign_exp_t);
    exp->base.node_type = AST_AssignExp;
    exp->base.type = NULL;
    exp->left = create_ast_exp(children[0]);
    exp->right = create_ast_exp(children[2]);
    return (ast_exp_t *) exp;
}

ast_exp_t * create_ast_call_exp(nt_cst_node_t * cst_node) {
    cst_node_t ** children = (cst_node_t **)(cst_node + 1);
    assert(cst_node->base.node_type == CST_Exp &&
           children[0]->node_type == CST_ID &&
           children[1]->node_type == CST_LP &&
           children[cst_node->length - 1]->node_type == CST_RP &&
           (cst_node->length == 3 || (cst_node->length == 4 && children[2]->node_type == CST_Args)));
    ast_call_exp_t * exp = NEW(ast_call_exp_t);
    exp->base.node_type = AST_CallExp;
    exp->base.type = NULL;
    exp->callee = create_ast_id_exp((str_cst_node_t *) children[0]);
    exp->arguments = create_array(ast_exp_t_ptr, 0, 1);
    if (cst_node->length == 4) {
        nt_cst_node_t * args;
        for (args = (nt_cst_node_t *) children[2];
             args->base.node_type == CST_Args && args->length == 3;
             args = (nt_cst_node_t *) children[2]
        ) {
            children = (cst_node_t **)(args + 1);
            assert(children[1]->node_type == CST_COMMA);
            append(ast_exp_t_ptr, &(exp->arguments), create_ast_exp(children[0]));
        }
        assert(args->base.node_type == CST_Args && args->length == 1);
        append(ast_exp_t_ptr, &(exp->arguments), create_ast_exp((cst_node_t *)(args + 1)));
    }
    return (ast_exp_t *) exp;
}

ast_exp_t * create_ast_exp(cst_node_t * cst_node) {
    switch (cst_node->node_type) {
    case CST_ID:
        return create_ast_id_exp((str_cst_node_t *) cst_node);
    case CST_CHAR:
    case CST_INT:
    case CST_FLOAT:
        return create_ast_literal_exp(cst_node);
    case CST_Exp: {
        nt_cst_node_t * node = (nt_cst_node_t *) cst_node;
        cst_node_t ** children = (cst_node_t **)(node + 1);
        switch (node->length) {
        case 1:
            return create_ast_exp(children[0]);
        case 2:
            return create_ast_binary_exp(node);
        case 3:
            switch (children[1]->node_type) {
            case CST_ASSIGN:    return create_ast_assign_exp(node);
            case CST_Exp:       return create_ast_exp(children[1]);
            case CST_LP:        return create_ast_call_exp(node);
            case CST_DOT:       return create_ast_member_exp(node);
            default:            return create_ast_binary_exp(node);
            }
        case 4:
            if (children[1]->node_type == CST_LP) return create_ast_call_exp(node);
            else return create_ast_array_exp(node);
        }
    }
    default:
        return NULL;
    }
}


/* Statements */
ast_stmt_t * create_ast_exp_stmt(nt_cst_node_t * node) {
    cst_node_t ** children = (cst_node_t **)(node + 1);
    assert(node->base.node_type == CST_Stmt &&
           node->length == 2 &&
           children[0]->node_type == CST_Exp &&
           children[1]->node_type == CST_SEMI);
    ast_exp_stmt_t * stmt = NEW(ast_exp_stmt_t);
    stmt->base.node_type = AST_ExpStmt;
    stmt->expression = create_ast_exp(children[0]);
    return (ast_stmt_t *) stmt;
}

ast_stmt_t * create_ast_return_stmt(nt_cst_node_t * node) {
    cst_node_t ** children = (cst_node_t **)(node + 1);
    assert(node->base.node_type == CST_Stmt &&
           node->length == 3 &&
           children[0]->node_type == CST_RETURN &&
           children[1]->node_type == CST_Exp &&
           children[2]->node_type == CST_SEMI);
    ast_return_stmt_t * stmt = NEW(ast_return_stmt_t);
    stmt->base.node_type = AST_ReturnStmt;
    stmt->argument = create_ast_exp(children[1]);
    return (ast_stmt_t *) stmt;
}

ast_stmt_t * create_ast_if_stmt(nt_cst_node_t * node) {
    cst_node_t ** children = (cst_node_t **)(node + 1);
    assert(node->base.node_type == CST_Stmt &&
           (node->length == 5 ||
            (node->length == 7 &&
             children[5]->node_type == CST_ELSE &&
             children[6]->node_type == CST_Stmt)) &&
           children[0]->node_type == CST_IF &&
           children[1]->node_type == CST_LP &&
           children[2]->node_type == CST_Exp &&
           children[3]->node_type == CST_RP &&
           children[4]->node_type == CST_Stmt);
    ast_if_stmt_t * stmt = NEW(ast_if_stmt_t);
    stmt->base.node_type = AST_IfStmt;
    stmt->test = create_ast_exp(children[2]);
    stmt->consequent = create_ast_stmt((nt_cst_node_t *)(children[4]));
    stmt->alternate = node->length == 7 ? create_ast_stmt((nt_cst_node_t *)children[6]) : NULL;
    return (ast_stmt_t *) stmt;
}

ast_stmt_t * create_ast_while_stmt(nt_cst_node_t * node) {
    cst_node_t ** children = (cst_node_t **)(node + 1);
    assert(node->base.node_type == CST_Stmt &&
           node->length == 5 &&
           children[0]->node_type == CST_WHILE &&
           children[1]->node_type == CST_LP &&
           children[2]->node_type == CST_Exp &&
           children[3]->node_type == CST_RP &&
           children[4]->node_type == CST_Stmt);
    ast_while_stmt_t * stmt = NEW(ast_while_stmt_t);
    stmt->base.node_type = AST_WhileStmt;
    stmt->test = create_ast_exp(children[2]);
    stmt->body = create_ast_stmt((nt_cst_node_t *)children[4]);
    return (ast_stmt_t *) stmt;
}

ast_stmt_t * create_ast_for_stmt(nt_cst_node_t * node) {
    cst_node_t ** children = (cst_node_t **)(node + 1);
    assert(node->base.node_type == CST_Stmt &&
           6 <= node->length && node->length <= 9 &&
           children[0]->node_type == CST_FOR &&
           children[node->length - 1]->node_type == CST_Stmt);
    ast_for_stmt_t * stmt = NEW(ast_for_stmt_t);
    stmt->base.node_type = AST_ForStmt;
    switch (node->length) {
    case 6:
        stmt->init = stmt->test = stmt->update = NULL;
        break;
    case 7:
        if (children[2]->node_type == CST_Exp) {
            stmt->init = create_ast_exp(children[2]);
            stmt->test = stmt->update = NULL;
        } else if (children[3]->node_type == CST_Exp) {
            stmt->test = create_ast_exp(children[3]);
            stmt->init = stmt->update = NULL;
        } else { // children[4]->node_type == CST_Exp
            stmt->update = create_ast_exp(children[4]);
            stmt->init = stmt->test = NULL;
        }
        break;
    case 8:
        if (children[5]->node_type == CST_SEMI) {
            stmt->init = create_ast_exp(children[2]);
            stmt->test = create_ast_exp(children[4]);
            stmt->update = NULL;
        } else if (children[4]->node_type == CST_SEMI) {
            stmt->init = create_ast_exp(children[2]);
            stmt->update = create_ast_exp(children[5]);
            stmt->test = NULL;
        } else {
            stmt->test = create_ast_exp(children[3]);
            stmt->update = create_ast_exp(children[5]);
            stmt->init = NULL;
        }
        break;
    case 9:
        stmt->init = create_ast_exp(children[2]);
        stmt->test = create_ast_exp(children[4]);
        stmt->update = create_ast_exp(children[6]);
        break;
    default: break;
    }
    stmt->body = create_ast_stmt((nt_cst_node_t *) children[node->length - 1]);
    return (ast_stmt_t *) stmt;
}

ast_stmt_t * create_ast_comp_stmt(nt_cst_node_t * node) {
    cst_node_t ** children = (cst_node_t **)(node + 1);
    assert(node->base.node_type == CST_CompSt &&
           node->length == 4 &&
           children[0]->node_type == CST_LC &&
           children[1]->node_type == CST_DefList &&
           children[2]->node_type == CST_StmtList &&
           children[3]->node_type == CST_RC);
    ast_comp_stmt_t * stmt = NEW(ast_comp_stmt_t);
    stmt->base.node_type = AST_CompStmt;

    // TODO: Process DefList
    stmt->definitions = create_array(ast_var_def_t_ptr, 0, 1);
    nt_cst_node_t * list;
    nt_cst_node_t ** sub;

    // Process StmtList
    stmt->body = create_array(ast_stmt_t_ptr, 0, 1);
    for (list = (nt_cst_node_t *) children[2];
         list->base.node_type == CST_StmtList && list->length == 2;
         list = sub[1]
    ) {
        sub = (nt_cst_node_t **)(list + 1);
        append(ast_stmt_t_ptr, &(stmt->body), create_ast_stmt(sub[0]));
    }
    return (ast_stmt_t *) stmt;
}

ast_stmt_t * create_ast_stmt(nt_cst_node_t * node) {
    assert(node->base.node_type == CST_Stmt);
    cst_node_t ** children = (cst_node_t **)(node + 1);
    switch (children[0]->node_type) {
    case CST_Exp:
        return create_ast_exp_stmt(node);
    case CST_CompSt:
        return create_ast_comp_stmt((nt_cst_node_t *) children[0]);
    case CST_RETURN:
        return create_ast_return_stmt(node);
    case CST_IF:
        return create_ast_if_stmt(node);
    case CST_WHILE:
        return create_ast_while_stmt(node);
    case CST_FOR:
        return create_ast_for_stmt(node);
    default:
        return NULL;
    }
}


/* Definitions */

static char * process_VarDec(type_t ** type_ptr, nt_cst_node_t * var_dec) {
    assert(var_dec->base.node_type == CST_VarDec &&
           (var_dec->length == 1 || var_dec->length == 4));
    cst_node_t ** sub = (cst_node_t **)(var_dec + 1);
    if (var_dec->length == 4) {
        *type_ptr = create_array_type(*type_ptr, ((int_cst_node_t *) sub[2])->value);
        return process_VarDec(type_ptr, (nt_cst_node_t *) sub[0]);
    } else {
        assert(sub[0]->node_type == CST_ID);
        return strdup(((str_cst_node_t *) sub[0])->value);
    }
}

static ast_var_def_t * create_ast_var_def_from_Dec(nt_cst_node_t * dec, type_t * type) {
    cst_node_t ** sub = (cst_node_t **)(dec + 1);
    assert(dec->base.node_type == CST_Dec && (dec->length == 1 || dec->length == 3) && type != NULL);

    ast_var_def_t * def = NEW(ast_var_def_t);
    def->base.node_type = AST_VariableDef;
    def->base.type = type;
    def->base.identifier = process_VarDec(&(def->base.type), (nt_cst_node_t *) sub[0]);
    def->init = dec->length == 3 ? create_ast_exp(sub[2]) : NULL;

    return def;
}

#ifndef SYNTAX_ERRS_H
#define SYNTAX_ERRS_H

typedef enum {
    SYNTAX_ERR_MISSING_SEMI,
    SYNTAX_ERR_MISSING_RP,
    SYNTAX_ERR_DEC_STMT_ORDER,
} syntax_err_t;

const char * syntax_err_msg(syntax_err_t err);

#endif
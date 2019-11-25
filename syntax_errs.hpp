#ifndef SYNTAX_ERRS_H
#define SYNTAX_ERRS_H

typedef enum {
    SYNTAX_ERR_MISSING_SEMI,
    SYNTAX_ERR_MISSING_RP,
    SYNTAX_ERR_MISSING_LP,
    SYNTAX_ERR_MISSING_RB,
    // SYNTAX_ERR_MISSING_RC,
    SYNTAX_ERR_DEC_STMT_ORDER,
} SyntaxErr;


const char * syntaxErrMsgs[] = {
    "Missing semicolon ';'",
    "Missing closing parenthesis ')'",
    "Missing opening parenthesis '('",
    "Missing closing bracket ']'",
    // "Missing closing brace '}'",
    "Statement precede declaration",
};

#endif
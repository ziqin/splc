#ifndef SYNTAX_ERR_HPP
#define SYNTAX_ERR_HPP

enum SyntaxErr {
    SYNTAX_ERR_MISSING_SEMI,
    SYNTAX_ERR_MISSING_RP,
    SYNTAX_ERR_MISSING_LP,
    SYNTAX_ERR_MISSING_RB,
    // SYNTAX_ERR_MISSING_RC,
    SYNTAX_ERR_DEC_STMT_ORDER,
};


const char * syntaxErrMsgs[] = {
    "Missing semicolon ';'",
    "Missing closing parenthesis ')'",
    "Missing opening parenthesis '('",
    "Missing closing bracket ']'",
    // "Missing closing brace '}'",
    "Statement precede declaration",
};

#endif // SYNTAX_ERR_HPP
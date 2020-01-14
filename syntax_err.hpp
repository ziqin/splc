#ifndef SYNTAX_ERR_HPP
#define SYNTAX_ERR_HPP

enum class SyntaxErr {
    MISSING_SEMI,
    MISSING_RP,
    MISSING_LP,
    MISSING_RB,
    // MISSING_RC,
    DEC_STMT_ORDER,
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

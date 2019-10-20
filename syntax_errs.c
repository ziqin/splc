#include "syntax_errs.h"

const char * syntax_err_msg(syntax_err_t err) {
    static const char * err_msgs[] = {
        "Missing semicolon ';'",
        "Missing closing parenthesis ')'",
        "Missing opening parenthesis '('",
        "Missing closing bracket ']'",
        // "Missing closing brace '}'",
        "Statement precede declaration",
    };
    return err_msgs[err];
}

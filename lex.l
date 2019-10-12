%{
#include <stdbool.h>
#include <stdlib.h>
#include "syntax.tab.h"

int yycolno = 1;
#define YY_USER_ACTION              \
    yylloc.first_line = yylineno;   \
    yylloc.first_column = yycolno;  \
    yylloc.last_line = yylineno;    \
    yycolno += yyleng;              \
    yylloc.last_column = yycolno;
%}

%option yylineno

digit       [0-9]
hex         [0-9A-Fa-f]
letter      [A-Za-z]
whitespace  (" "|\t)
newline     (\n|\r|\r\n)

INT         (0|[1-9]{digit}{0,9}|0(X|x)(0|[1-9A-Fa-f]{hex}{0,7}))
FLOAT       {digit}+\.{digit}+
CHAR        ('[^']'|'\\x{hex}{1,2}')
TYPE        (int|float|char)
ID          ({letter}|_)({letter}|{digit}|_){0,31}

fake_dec    {digit}+
fake_hex    0(X|x)({digit}|{letter})*
fake_char   (''|'\\x({digit}|{letter})*')
known_err   ({fake_dec}|{fake_hex}|{fake_char})

%%
{INT}           { yylval.int_value = atoi(yytext); return INT; }
{FLOAT}         { yylval.float_value = atof(yytext); return FLOAT; }
{CHAR}          { yylval.string_value = yytext; return CHAR; }
{TYPE}          { yylval.string_value = yytext; return TYPE; }
struct          { return STRUCT; }
if              { return IF; }
else            { return ELSE; }
while           { return WHILE; }
return          { return RETURN; }
{ID}            { yylval.string_value = yytext; return ID; }
"."             { return DOT; }
";"             { return SEMI; }
","             { return COMMA; }
"="             { return ASSIGN; }
"<"             { return LT; }
"<="            { return LE; }
">"             { return GT; }
">="            { return GE; }
"!="            { return NE; }
"=="            { return EQ; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MUL; }
"/"             { return DIV; }
"&&"            { return AND; }
"||"            { return OR; }
"!"             { return NOT; }
"("             { return LP; }
")"             { return RP; }
"["             { return LB; }
"]"             { return RB; }
"{"             { return LC; }
"}"             { return RC; }

{whitespace}+   {}
{newline}       { yycolno = 1; }

{known_err}     {}
.               {}
%%

// TODO:
// {known_err}     { yylval.string_value = yytext; return LEX_ERR; }
// .               { yylval.string_value = yytext; return LEX_ERR; }
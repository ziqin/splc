%{
#include <stdbool.h>
#include <stdlib.h>
#include "ast.h"
#include "syntax.tab.h"

int yycolno = 1;

#define YY_USER_ACTION              \
    yylloc.first_line = yylineno;   \
    yylloc.first_column = yycolno;  \
    yylloc.last_line = yylineno;    \
    yycolno += yyleng;              \
    yylloc.last_column = yycolno;

void lex_complain(int lineno, const char * token);
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
{INT}           { yylval = create_int_ast_node(AST_INT, atoi(yytext)); return INT; }
{FLOAT}         { yylval = create_float_ast_node(AST_FLOAT, atof(yytext)); return FLOAT; }
{CHAR}          { yylval = create_str_ast_node(AST_CHAR, yytext); return CHAR; }
{TYPE}          { yylval = create_str_ast_node(AST_TYPE, yytext); return TYPE; }
struct          { yylval = create_ast_node(AST_STRUCT); return STRUCT; }
if              { yylval = create_ast_node(AST_IF); return IF; }
else            { yylval = create_ast_node(AST_ELSE); return ELSE; }
while           { yylval = create_ast_node(AST_WHILE); return WHILE; }
return          { yylval = create_ast_node(AST_RETURN); return RETURN; }
{ID}            { yylval = create_str_ast_node(AST_ID, yytext); return ID; }
"."             { yylval = create_ast_node(AST_DOT); return DOT; }
";"             { yylval = create_ast_node(AST_SEMI); return SEMI; }
","             { yylval = create_ast_node(AST_COMMA); return COMMA; }
"="             { yylval = create_ast_node(AST_ASSIGN); return ASSIGN; }
"<"             { yylval = create_ast_node(AST_LT); return LT; }
"<="            { yylval = create_ast_node(AST_LE); return LE; }
">"             { yylval = create_ast_node(AST_GT); return GT; }
">="            { yylval = create_ast_node(AST_GE); return GE; }
"!="            { yylval = create_ast_node(AST_NE); return NE; }
"=="            { yylval = create_ast_node(AST_EQ); return EQ; }
"+"             { yylval = create_ast_node(AST_PLUS); return PLUS; }
"-"             { yylval = create_ast_node(AST_MINUS); return MINUS; }
"*"             { yylval = create_ast_node(AST_MUL); return MUL; }
"/"             { yylval = create_ast_node(AST_DIV); return DIV; }
"&&"            { yylval = create_ast_node(AST_AND); return AND; }
"||"            { yylval = create_ast_node(AST_OR); return OR; }
"!"             { yylval = create_ast_node(AST_NOT); return NOT; }
"("             { yylval = create_ast_node(AST_LP); return LP; }
")"             { yylval = create_ast_node(AST_RP); return RP; }
"["             { yylval = create_ast_node(AST_LB); return LB; }
"]"             { yylval = create_ast_node(AST_RB); return RB; }
"{"             { yylval = create_ast_node(AST_LC); return LC; }
"}"             { yylval = create_ast_node(AST_RC); return RC; }

{whitespace}+   {}
{newline}       { yycolno = 1; }

{known_err}     { lex_complain(yylineno, yytext); return LEX_ERR; }
.               { lex_complain(yylineno, yytext); return LEX_ERR; }
%%

void lex_complain(int lineno, const char * token) {
    fprintf(stderr, "Error type A at Line %d: Mysterious lexeme %s\n", lineno, token);
}
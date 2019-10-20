%{
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ast.h"
#include "syntax.tab.h"

static int yycolno = 1;
static int prev_state;

#define YY_USER_ACTION              \
    yylloc.first_line = yylineno;   \
    yylloc.first_column = yycolno;  \
    yylloc.last_line = yylineno;    \
    yycolno += yyleng;              \
    yylloc.last_column = yycolno;

void report_lex_err(int, const char *, ...);

%}

%option yylineno
%x BLK_COMMENT

digit           [0-9]
hex             [0-9A-Fa-f]
letter          [A-Za-z]
whitespace      (" "|\t)
newline         (\n|\r|\r\n)

INT             (0|[1-9]{digit}{0,9}|0(X|x)(0|[1-9A-Fa-f]{hex}{0,7}))
fraction        {digit}+\.{digit}+
scientific      {digit}+(\.{digit}+)?(E|e)("+"|"-")?{digit}+
FLOAT           ({fraction}|{scientific})
CHAR            ('[^']'|'\\x{hex}{1,2}')
TYPE            (int|float|char)
ID              ({letter}|_)({letter}|{digit}|_){0,31}
line_cmmt       "//"[^\r\n]*
blk_cmmt_begin  "/*"
blk_cmmt_end    "*/"

fake_dec    {digit}+
fake_hex    0(X|x)({digit}|{letter})*
fake_char   (''|'\\x({digit}|{letter})*')
fake_id     {digit}({letter}|{digit}|_)+
known_err   ({fake_dec}|{fake_hex}|{fake_char}|{fake_id})

%%
{INT}           { yylval = create_int_ast_node(AST_INT, atoi(yytext)); return INT; }
{FLOAT}         { yylval = create_float_ast_node(AST_FLOAT, atof(yytext)); return FLOAT; }
{CHAR}          { yylval = create_str_ast_node(AST_CHAR, yytext); return CHAR; }
{TYPE}          { yylval = create_str_ast_node(AST_TYPE, yytext); return TYPE; }
struct          { yylval = create_ast_node(AST_STRUCT); return STRUCT; }
if              { yylval = create_ast_node(AST_IF); return IF; }
else            { yylval = create_ast_node(AST_ELSE); return ELSE; }
while           { yylval = create_ast_node(AST_WHILE); return WHILE; }
for             { yylval = create_ast_node(AST_FOR); return FOR; }
return          { yylval = create_ast_node(AST_RETURN); return RETURN; }
{ID}            { yylval = create_str_ast_node(AST_ID, yytext); return ID; }  // the order matters
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

{whitespace}+   ;
{line_cmmt}     ;
{newline}       { yycolno = 1; }  // reset

{blk_cmmt_begin}                { prev_state = YYSTATE; BEGIN BLK_COMMENT; }
{blk_cmmt_end}                  { report_lex_err(yylineno, "Illegal block comment"); return LEX_ERR_BLK; }
<BLK_COMMENT>{blk_cmmt_begin}   { report_lex_err(yylineno, "Illegal block comment"); return LEX_ERR_BLK; }
<BLK_COMMENT>{blk_cmmt_end}     { BEGIN prev_state; }
<BLK_COMMENT>(.|{newline})      ;

{known_err}|.   { report_lex_err(yylineno, "unknown lexeme %s", yytext); return LEX_ERR; }
%%

void report_lex_err(int lineno, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Error type A at Line %d: ", lineno);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
}
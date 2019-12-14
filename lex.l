%{

#include <cstdarg>
#include <cstdio>
#include "ast.hpp"
#include "syntax.tab.h"

static int yycolno = 1;
static int prev_state;

#define YY_USER_ACTION              \
    yylloc.first_line = yylineno;   \
    yylloc.first_column = yycolno;  \
    yylloc.last_line = yylineno;    \
    yycolno += yyleng;              \
    yylloc.last_column = yycolno;

static void reportLexErr(int, const char *, ...);
static char str2char(const char * val);

extern "C" int yylex(void);

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
{CHAR}          { yylval.CHAR = str2char(yytext); return CHAR; }
{INT}           { yylval.INT = std::atoi(yytext); return INT; }
{FLOAT}         { yylval.FLOAT = std::atof(yytext); return FLOAT; }
{TYPE}          { yylval.TYPE = new std::string(yytext); return TYPE; }
struct          { return STRUCT; }
if              { return IF; }
else            { return ELSE; }
while           { return WHILE; }
for             { return FOR; }
return          { return RETURN; }
{ID}            { yylval.ID = new std::string(yytext); return ID; }  // the order matters
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

{whitespace}+   ;
{line_cmmt}     ;
{newline}       { yycolno = 1; }  // reset

{blk_cmmt_begin}                { prev_state = YYSTATE; BEGIN BLK_COMMENT; }
{blk_cmmt_end}                  { reportLexErr(yylineno, "Illegal block comment"); return LEX_ERR_BLK; }
<BLK_COMMENT>{blk_cmmt_begin}   { reportLexErr(yylineno, "Illegal block comment"); return LEX_ERR_BLK; }
<BLK_COMMENT>{blk_cmmt_end}     { BEGIN prev_state; }
<BLK_COMMENT>(.|{newline})      ;

{known_err}|.   { reportLexErr(yylineno, "unknown lexeme %s", yytext); return LEX_ERR; }
%%

static void reportLexErr(int lineno, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Error type A at Line %d: ", lineno);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
}

static char str2char(const char * val) {
    char value;
    if (val[1] != '\\') {
        value = val[1];
    } else { // starts with \x
        value = 0;
        for (const char * hex = val + 3; *hex != '\''; ++hex) {
            value <<= 4;
            if ('0' <= *hex && *hex <= '9') {
                value += *hex - '0';
            } else if ('A' <= *hex && *hex <= 'F') {
                value += *hex - 'A' + 10;
            } else if ('a' <= *hex && *hex <= 'f') {
                value += *hex - 'a' + 10;
            }
        }
    }
    return value;
}

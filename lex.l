%{
    #include <cstdarg>
    #include <cstdio>
    #include "cst.hpp"
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
{CHAR}          { yylval = new CST::CharNode(yytext); return CHAR; }
{INT}           { yylval = new CST::IntNode(yytext); return INT; }
{FLOAT}         { yylval = new CST::FloatNode(yytext); return FLOAT; }
{TYPE}          { yylval = new CST::StrNode(CST::TYPE, yytext); return TYPE; }
struct          { yylval = new CST::Node(CST::STRUCT); return STRUCT; }
if              { yylval = new CST::Node(CST::IF); return IF; }
else            { yylval = new CST::Node(CST::ELSE); return ELSE; }
while           { yylval = new CST::Node(CST::WHILE); return WHILE; }
for             { yylval = new CST::Node(CST::FOR); return FOR; }
return          { yylval = new CST::Node(CST::RETURN); return RETURN; }
{ID}            { yylval = new CST::StrNode(CST::ID, yytext); return ID; }  // the order matters
"."             { yylval = new CST::Node(CST::DOT); return DOT; }
";"             { yylval = new CST::Node(CST::SEMI); return SEMI; }
","             { yylval = new CST::Node(CST::COMMA); return COMMA; }
"="             { yylval = new CST::Node(CST::ASSIGN); return ASSIGN; }
"<"             { yylval = new CST::Node(CST::LT); return LT; }
"<="            { yylval = new CST::Node(CST::LE); return LE; }
">"             { yylval = new CST::Node(CST::GT); return GT; }
">="            { yylval = new CST::Node(CST::GE); return GE; }
"!="            { yylval = new CST::Node(CST::NE); return NE; }
"=="            { yylval = new CST::Node(CST::EQ); return EQ; }
"+"             { yylval = new CST::Node(CST::PLUS); return PLUS; }
"-"             { yylval = new CST::Node(CST::MINUS); return MINUS; }
"*"             { yylval = new CST::Node(CST::MUL); return MUL; }
"/"             { yylval = new CST::Node(CST::DIV); return DIV; }
"&&"            { yylval = new CST::Node(CST::AND); return AND; }
"||"            { yylval = new CST::Node(CST::OR); return OR; }
"!"             { yylval = new CST::Node(CST::NOT); return NOT; }
"("             { yylval = new CST::Node(CST::LP); return LP; }
")"             { yylval = new CST::Node(CST::RP); return RP; }
"["             { yylval = new CST::Node(CST::LB); return LB; }
"]"             { yylval = new CST::Node(CST::RB); return RB; }
"{"             { yylval = new CST::Node(CST::LC); return LC; }
"}"             { yylval = new CST::Node(CST::RC); return RC; }

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

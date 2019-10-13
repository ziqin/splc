%{
#include <stdbool.h>
#include "parser.h"

int yylex(void);
void yyerror(const char *);

FILE *yyin;
static AstNode * program;
bool has_lex_err;
%}

%locations

%define api.value.type {AstNode *}

%token INT FLOAT TYPE ID CHAR STRUCT IF ELSE WHILE RETURN DOT SEMI COMMA ASSIGN LT LE GT GE NE EQ PLUS MINUS MUL DIV AND OR NOT LP RP LB RB LC RC
%token LEX_ERR

%nonassoc LOWER_ELSE
%nonassoc ELSE
%right ASSIGN
%left OR
%left AND
%left LT LE GT GE EQ NE
%left PLUS MINUS
%left MUL DIV
%right UMINUS NOT
%left LP RP LB RB DOT

%%
/* high-level definition */
Program: ExtDefList                 { program = $$ = create_nt_ast_node(AST_Program, &@$, 1, $1); }
    ;
ExtDefList: ExtDef ExtDefList       { $$ = create_nt_ast_node(AST_ExtDefList, &@$, 2, $1, $2); }
    | /* empty */                   { $$ = create_nt_ast_node(AST_ExtDefList, &@$, 0); }
    ;
ExtDef: Specifier ExtDecList SEMI   { $$ = create_nt_ast_node(AST_ExtDef, &@$, 3, $1, $2, $3); }
    | Specifier SEMI                { $$ = create_nt_ast_node(AST_ExtDef, &@$, 2, $1, $2); }
    | Specifier FunDec CompSt       { $$ = create_nt_ast_node(AST_ExtDef, &@$, 3, $1, $2, $3); }
    ;
ExtDecList: VarDec                  { $$ = create_nt_ast_node(AST_ExtDecList, &@$, 1, $1); }
    | VarDec COMMA ExtDecList       { $$ = create_nt_ast_node(AST_ExtDecList, &@$, 3, $1, $2, $3); }
    ;

/* specifier */
Specifier: TYPE                             { $$ = create_nt_ast_node(AST_Specifier, &@$, 1, $1); }
    | StructSpecifier                       { $$ = create_nt_ast_node(AST_Specifier, &@$, 1, $1); }
    ;
StructSpecifier: STRUCT ID LC DefList RC    { $$ = create_nt_ast_node(AST_StructSpecifier, &@$, 5, $1, $2, $3, $4, $5); }
    | STRUCT ID                             { $$ = create_nt_ast_node(AST_StructSpecifier, &@$, 2, $1, $2); }
    ;

/* declarator */
VarDec: ID                      { $$ = create_nt_ast_node(AST_VarDec, &@$, 1, $1); }
    | VarDec LB INT RB          { $$ = create_nt_ast_node(AST_VarDec, &@$, 4, $1, $2, $3, $4); }
    ;
FunDec: ID LP VarList RP        { $$ = create_nt_ast_node(AST_FunDec, &@$, 4, $1, $2, $3, $4); }
    | ID LP RP                  { $$ = create_nt_ast_node(AST_FunDec, &@$, 3, $1, $2, $3); }    
    ;
VarList: ParamDec COMMA VarList { $$ = create_nt_ast_node(AST_VarList, &@$, 3, $1, $2, $3); }
    | ParamDec                  { $$ = create_nt_ast_node(AST_VarList, &@$, 1, $1); }
    ;
ParamDec: Specifier VarDec      { $$ = create_nt_ast_node(AST_ParamDec, &@$, 2, $1, $2); }
    ;

/* statement */
CompSt: LC DefList StmtList RC              { $$ = create_nt_ast_node(AST_CompSt, &@$, 4, $1, $2, $3, $4); }
    ;
StmtList: Stmt StmtList                     { $$ = create_nt_ast_node(AST_StmtList, &@$, 2, $1, $2); }
    | /* empty */                           { $$ = create_nt_ast_node(AST_StmtList, &@$, 0); }
    ;
Stmt: Exp SEMI                              { $$ = create_nt_ast_node(AST_Stmt, &@$, 2, $1, $2); }
    | CompSt                                { $$ = create_nt_ast_node(AST_Stmt, &@$, 1, $1); }
    | RETURN Exp SEMI                       { $$ = create_nt_ast_node(AST_Stmt, &@$, 3, $1, $2, $3); }
    | IF LP Exp RP Stmt %prec LOWER_ELSE    { $$ = create_nt_ast_node(AST_Stmt, &@$, 5, $1, $2, $3, $4, $5); }
    | IF LP Exp RP Stmt ELSE Stmt           { $$ = create_nt_ast_node(AST_Stmt, &@$, 7, $1, $2, $3, $4, $5, $6, $7); }
    | WHILE LP Exp RP Stmt                  { $$ = create_nt_ast_node(AST_Stmt, &@$, 5, $1, $2, $3, $4, $5); }
    ;

/* local definition */
DefList: Def DefList        { $$ = create_nt_ast_node(AST_DefList, &@$, 2, $1, $2); }
    | /* empty */           { $$ = create_nt_ast_node(AST_DefList, &@$, 0); }
    ;
Def: Specifier DecList SEMI { $$ = create_nt_ast_node(AST_Def, &@$, 3, $1, $2, $3); }
    ;
DecList: Dec                { $$ = create_nt_ast_node(AST_DecList, &@$, 1, $1); }
    | Dec COMMA DecList     { $$ = create_nt_ast_node(AST_DecList, &@$, 3, $1, $2, $3); }
    ;   
Dec: VarDec                 { $$ = create_nt_ast_node(AST_Dec, &@$, 1, $1); }
    | VarDec ASSIGN Exp     { $$ = create_nt_ast_node(AST_Dec, &@$, 3, $1, $2, $3); }
    ;

/* Expression */
Exp: Exp ASSIGN Exp                 { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp AND Exp                   { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp OR Exp                    { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp LT Exp                    { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp LE Exp                    { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp GT Exp                    { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp GE Exp                    { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp NE Exp                    { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp EQ Exp                    { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp PLUS Exp                  { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp MINUS Exp                 { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp MUL Exp                   { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp DIV Exp                   { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | LP Exp RP                     { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | MINUS Exp     %prec UMINUS    { $$ = create_nt_ast_node(AST_Exp, &@$, 2, $1, $2); }
    | NOT Exp                       { $$ = create_nt_ast_node(AST_Exp, &@$, 2, $1, $2); }
    | ID LP Args RP                 { $$ = create_nt_ast_node(AST_Exp, &@$, 4, $1, $2, $3, $4); }
    | ID LP RP                      { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | Exp LB Exp RB                 { $$ = create_nt_ast_node(AST_Exp, &@$, 4, $1, $2, $3, $4); }
    | Exp DOT ID    %prec DOT       { $$ = create_nt_ast_node(AST_Exp, &@$, 3, $1, $2, $3); }
    | ID                            { $$ = create_nt_ast_node(AST_Exp, &@$, 1, $1); }
    | INT                           { $$ = create_nt_ast_node(AST_Exp, &@$, 1, $1); }
    | FLOAT                         { $$ = create_nt_ast_node(AST_Exp, &@$, 1, $1); }
    | CHAR                          { $$ = create_nt_ast_node(AST_Exp, &@$, 1, $1); }
    | LEX_ERR                       { $$ = NULL; has_lex_err = true; }
    ;

Args: Exp COMMA Args    { $$ = create_nt_ast_node(AST_Args, &@$, 3, $1, $2, $3); }
    | Exp               { $$ = create_nt_ast_node(AST_Args, &@$, 1, $1); }
    ;

%%

// void yyerror(YYLTYPE *locp, const char * msg) {
//     fprintf(stderr, "Error at Line %d: %s\n", locp->first_line, locpmsg);
// }

void yyerror(const char * msg) {
    fprintf(stderr, "Error type B at Line %d: %s\n", yylloc.first_line, msg);
}

AstNode * build_ast(FILE * file) {
    // yydebug = 1;
    has_lex_err = false;
    yyin = file;
    return yyparse() == 0 && !has_lex_err ? program : (delete_ast_node(program), NULL);
}

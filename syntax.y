%{
#include <stdbool.h>
#include "parser.h"
#include "syntax_errs.h"

int yylex(void);
void yyerror(const char *);
void report_err(int, syntax_err_t);

FILE * yyin;
static cst_node_t * program;
static bool has_err;

%}

%locations

%define api.value.type {cst_node_t *}

%token INT FLOAT TYPE ID CHAR STRUCT IF ELSE WHILE FOR RETURN DOT SEMI COMMA ASSIGN LT LE GT GE NE EQ PLUS MINUS MUL DIV AND OR NOT LP RP LB RB LC RC
%token LEX_ERR LEX_ERR_BLK

%nonassoc ERROR

%nonassoc LOWER_ELSE
%nonassoc ELSE

%nonassoc SEMI
%nonassoc ARGS
%right COMMA
%right ASSIGN
%left OR
%left AND
%left LT LE GT GE EQ NE
%left PLUS MINUS
%left MUL DIV
%right UPLUS UMINUS NOT
%nonassoc INT FLOAT CHAR ID LEX_ERR
%left LP RP LB RB DOT

%%
/* high-level definition */
Program: ExtDefList                 { program = $$ = create_nt_cst_node(CST_Program, &@$, 1, $1); }
    ;
ExtDefList: ExtDef ExtDefList       { $$ = create_nt_cst_node(CST_ExtDefList, &@$, 2, $1, $2); }
    | /* empty */                   { $$ = create_nt_cst_node(CST_ExtDefList, &@$, 0); }
    | LEX_ERR_BLK                   { $$ = NULL; has_err = true; }
    ;
ExtDef: Specifier ExtDecList SEMI            { $$ = create_nt_cst_node(CST_ExtDef, &@$, 3, $1, $2, $3); }
    | Specifier SEMI                         { $$ = create_nt_cst_node(CST_ExtDef, &@$, 2, $1, $2); }
    | Specifier FunDec CompSt                { $$ = create_nt_cst_node(CST_ExtDef, &@$, 3, $1, $2, $3); }
    | Specifier ExtDecList       %prec ERROR { $$ = NULL; report_err(@2.last_line, SYNTAX_ERR_MISSING_SEMI); }
    | Specifier                  %prec ERROR { $$ = NULL; report_err(@$.last_line, SYNTAX_ERR_MISSING_SEMI); }
    ;
ExtDecList: VarDec               { $$ = create_nt_cst_node(CST_ExtDecList, &@$, 1, $1); }
    | VarDec COMMA ExtDecList    { $$ = create_nt_cst_node(CST_ExtDecList, &@$, 3, $1, $2, $3); }
    ;

/* specifier */
Specifier: TYPE                             { $$ = create_nt_cst_node(CST_Specifier, &@$, 1, $1); }
    | StructSpecifier                       { $$ = create_nt_cst_node(CST_Specifier, &@$, 1, $1); }
    ;
StructSpecifier: STRUCT ID LC DefList RC    { $$ = create_nt_cst_node(CST_StructSpecifier, &@$, 5, $1, $2, $3, $4, $5); }
    | STRUCT ID                             { $$ = create_nt_cst_node(CST_StructSpecifier, &@$, 2, $1, $2); }
    // | STRUCT ID LC DefList      %prec ERROR { $$ = NULL; delete_cst_node($1); delete_cst_node($2); delete_cst_node($3); delete_cst_node($4); report_err(@4.last_line, SYNTAX_ERR_MISSING_RC); }
    ;

/* declarator */
VarDec: ID                      { $$ = create_nt_cst_node(CST_VarDec, &@$, 1, $1); }
    | VarDec LB INT RB          { $$ = create_nt_cst_node(CST_VarDec, &@$, 4, $1, $2, $3, $4); }
    | LEX_ERR       %prec ERROR { $$ = NULL; has_err = true; }
    | VarDec LB INT %prec ERROR { $$ = NULL; delete_cst_node($1); delete_cst_node($2); delete_cst_node($3); report_err(@3.last_line, SYNTAX_ERR_MISSING_RB); }
    ;
FunDec: ID LP VarList RP        { $$ = create_nt_cst_node(CST_FunDec, &@$, 4, $1, $2, $3, $4); }
    | ID LP RP                  { $$ = create_nt_cst_node(CST_FunDec, &@$, 3, $1, $2, $3); }
    | ID LP VarList %prec ERROR { $$ = NULL; delete_cst_node($1); delete_cst_node($2); delete_cst_node($3); report_err(@3.last_line, SYNTAX_ERR_MISSING_RP); }
    | ID LP         %prec ERROR { $$ = NULL; delete_cst_node($1); delete_cst_node($2); report_err(@2.last_line, SYNTAX_ERR_MISSING_RP); }
    // | ID VarList RP %prec ERROR { $$ = NULL; delete_cst_node($1); delete_cst_node($2); delete_cst_node($3); report_err(@2.first_line, SYNTAX_ERR_MISSING_LP); }
    | ID RP         %prec ERROR { $$ = NULL; delete_cst_node($1); delete_cst_node($2); report_err(@2.first_line, SYNTAX_ERR_MISSING_LP); }
    ;
VarList: ParamDec COMMA VarList { $$ = create_nt_cst_node(CST_VarList, &@$, 3, $1, $2, $3); }
    | ParamDec                  { $$ = create_nt_cst_node(CST_VarList, &@$, 1, $1); }
    ;
ParamDec: Specifier VarDec      { $$ = create_nt_cst_node(CST_ParamDec, &@$, 2, $1, $2); }
    ;

/* statement */
CompSt: LC DefList StmtList RC              { $$ = create_nt_cst_node(CST_CompSt, &@$, 4, $1, $2, $3, $4); }
    // | LC DefList StmtList       %prec ERROR { $$ = NULL; delete_cst_node($1); delete_cst_node($2); delete_cst_node($3); report_err(@3.last_line, SYNTAX_ERR_MISSING_RC); }
    ;
StmtList: Stmt StmtList                     { $$ = create_nt_cst_node(CST_StmtList, &@$, 2, $1, $2); }
    | /* empty */                           { $$ = create_nt_cst_node(CST_StmtList, &@$, 0); }
    | Stmt Def StmtList         %prec ERROR { $$ = NULL; delete_cst_node($1); delete_cst_node($2); report_err(@1.last_line, SYNTAX_ERR_DEC_STMT_ORDER); }
    ;
Stmt: Exp SEMI                              { $$ = create_nt_cst_node(CST_Stmt, &@$, 2, $1, $2); }
    | CompSt                                { $$ = create_nt_cst_node(CST_Stmt, &@$, 1, $1); }
    | RETURN Exp SEMI                       { $$ = create_nt_cst_node(CST_Stmt, &@$, 3, $1, $2, $3); }
    | IF LP Exp RP Stmt %prec LOWER_ELSE    { $$ = create_nt_cst_node(CST_Stmt, &@$, 5, $1, $2, $3, $4, $5); }
    | IF LP Exp RP Stmt ELSE Stmt           { $$ = create_nt_cst_node(CST_Stmt, &@$, 7, $1, $2, $3, $4, $5, $6, $7); }
    | WHILE LP Exp RP Stmt                  { $$ = create_nt_cst_node(CST_Stmt, &@$, 5, $1, $2, $3, $4, $5); }
    | FOR LP SEMI SEMI RP Stmt              { $$ = create_nt_cst_node(CST_Stmt, &@$, 9, $1, $2, NULL, $3, NULL, $4, NULL, $5, $6); }
    | FOR LP Exp SEMI SEMI RP Stmt          { $$ = create_nt_cst_node(CST_Stmt, &@$, 9, $1, $2, $3, $4, NULL, $5, NULL, $6, $7); }
    | FOR LP SEMI Exp SEMI RP Stmt          { $$ = create_nt_cst_node(CST_Stmt, &@$, 9, $1, $2, NULL, $3, $4, $5, NULL, $6, $7); }
    | FOR LP SEMI SEMI Exp RP Stmt          { $$ = create_nt_cst_node(CST_Stmt, &@$, 9, $1, $2, NULL, $3, NULL, $4, $5, $6, $7); }
    | FOR LP Exp SEMI Exp SEMI RP Stmt      { $$ = create_nt_cst_node(CST_Stmt, &@$, 9, $1, $2, $3, $4, $5, $6, NULL, $7, $8); }
    | FOR LP Exp SEMI SEMI Exp RP Stmt      { $$ = create_nt_cst_node(CST_Stmt, &@$, 9, $1, $2, $3, $4, NULL, $5, $6, $7, $8); }
    | FOR LP SEMI Exp SEMI Exp RP Stmt      { $$ = create_nt_cst_node(CST_Stmt, &@$, 9, $1, $2, NULL, $3, $4, $5, $6, $7, $8); }
    | FOR LP Exp SEMI Exp SEMI Exp RP Stmt  { $$ = create_nt_cst_node(CST_Stmt, &@$, 9, $1, $2, $3, $4, $5, $6, $7, $8, $9); }
    | Exp                       %prec ERROR { $$ = NULL; delete_cst_node($1); report_err(@$.last_line, SYNTAX_ERR_MISSING_SEMI); }
    | RETURN Exp                %prec ERROR { $$ = NULL; delete_cst_node($1); delete_cst_node($2); report_err(@2.last_line, SYNTAX_ERR_MISSING_SEMI); }
    | LEX_ERR_BLK               %prec ERROR { $$ = NULL; has_err = true; }
    | error SEMI                %prec ERROR { $$ = NULL; delete_cst_node($2); }
    // TODO:
    // | IF LP Exp Stmt      %prec LOWER_ERROR { $$ = NULL; report_err(@3.last_line, SYNTAX_ERR_MISSING_RP); }
    // | IF LP Exp Stmt ELSE Stmt  %prec ERROR { $$ = NULL; report_err(@3.last_line, SYNTAX_ERR_MISSING_RP); }
    // | WHILE LP Exp Stmt         %prec ERROR { $$ = NULL; report_err(@3.last_line, SYNTAX_ERR_MISSING_RP); }
    ;

/* local definition */
DefList: Def DefList            { $$ = create_nt_cst_node(CST_DefList, &@$, 2, $1, $2); }
    | /* empty */               { $$ = create_nt_cst_node(CST_DefList, &@$, 0); }
    ;
Def: Specifier DecList SEMI               { $$ = create_nt_cst_node(CST_Def, &@$, 3, $1, $2, $3); }
    | Specifier DecList       %prec ERROR { $$ = NULL; delete_cst_node($1); delete_cst_node($2); 
                                            report_err(@2.last_line, SYNTAX_ERR_MISSING_SEMI); }
    ;
DecList: Dec                    { $$ = create_nt_cst_node(CST_DecList, &@$, 1, $1); }
    | Dec COMMA DecList         { $$ = create_nt_cst_node(CST_DecList, &@$, 3, $1, $2, $3); }
    ;
Dec: VarDec                     { $$ = create_nt_cst_node(CST_Dec, &@$, 1, $1); }
    | VarDec ASSIGN Exp         { $$ = create_nt_cst_node(CST_Dec, &@$, 3, $1, $2, $3); }
    ;

/* Expression */
Exp: Exp ASSIGN Exp                 { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp AND Exp                   { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp OR Exp                    { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp LT Exp                    { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp LE Exp                    { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp GT Exp                    { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp GE Exp                    { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp NE Exp                    { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp EQ Exp                    { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp PLUS Exp                  { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp MINUS Exp                 { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp MUL Exp                   { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp DIV Exp                   { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | LP Exp RP                     { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | PLUS Exp         %prec UPLUS  { $$ = create_nt_cst_node(CST_Exp, &@$, 2, $1, $2); }
    | MINUS Exp        %prec UMINUS { $$ = create_nt_cst_node(CST_Exp, &@$, 2, $1, $2); }
    | NOT Exp          %prec NOT    { $$ = create_nt_cst_node(CST_Exp, &@$, 2, $1, $2); }
    | ID LP Args RP                 { $$ = create_nt_cst_node(CST_Exp, &@$, 4, $1, $2, $3, $4); }
    | ID LP RP                      { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | Exp LB Exp RB                 { $$ = create_nt_cst_node(CST_Exp, &@$, 4, $1, $2, $3, $4); }
    | Exp DOT ID                    { $$ = create_nt_cst_node(CST_Exp, &@$, 3, $1, $2, $3); }
    | ID                            { $$ = create_nt_cst_node(CST_Exp, &@$, 1, $1); }
    | INT                           { $$ = create_nt_cst_node(CST_Exp, &@$, 1, $1); }
    | FLOAT                         { $$ = create_nt_cst_node(CST_Exp, &@$, 1, $1); }
    | CHAR                          { $$ = create_nt_cst_node(CST_Exp, &@$, 1, $1); }
    | LEX_ERR                       { $$ = NULL; has_err = true; }
    | Exp LEX_ERR Exp  %prec ERROR  { $$ = NULL; has_err = true; }
    | LP Exp           %prec ERROR  { $$ = NULL; delete_cst_node($1); delete_cst_node($2); report_err(@2.last_line, SYNTAX_ERR_MISSING_RP); }
    | ID LP Args       %prec ERROR  { $$ = NULL; delete_cst_node($1); delete_cst_node($2); delete_cst_node($3); report_err(@3.last_line, SYNTAX_ERR_MISSING_RP); }
    | ID LP            %prec ERROR  { $$ = NULL; delete_cst_node($1); delete_cst_node($2); report_err(@2.last_line, SYNTAX_ERR_MISSING_RP); }
    | Exp LB Exp       %prec ERROR  { $$ = NULL; delete_cst_node($1); delete_cst_node($2); delete_cst_node($3); report_err(@3.last_line, SYNTAX_ERR_MISSING_RB); }
    // TODO:
    // | Exp RP           %prec ERROR  { $$ = NULL; delete_cst_node($1); delete_cst_node($2); report_err(@1.first_line, SYNTAX_ERR_MISSING_LP); }
    // | ID Args RP       %prec ERROR  { $$ = NULL; delete_cst_node($1); delete_cst_node($2); delete_cst_node($3); report_err(@1.last_line, SYNTAX_ERR_MISSING_LP); }
    // | ID RP            %prec ERROR  { $$ = NULL; delete_cst_node($1); delete_cst_node($2); report_err(@1.last_line, SYNTAX_ERR_MISSING_LP); }
    ;

Args: Exp COMMA Args    %prec ARGS { $$ = create_nt_cst_node(CST_Args, &@$, 3, $1, $2, $3); }
    | Exp               %prec ARGS { $$ = create_nt_cst_node(CST_Args, &@$, 1, $1); }
    ;

%%

void yyerror(const char * msg) {
    fprintf(stderr, "Error type B at Line %d: %s\n", yylloc.last_line, msg);
}

void report_err(int lineno, syntax_err_t err) {
    fprintf(stderr, "Error type B at Line %d: %s\n", lineno, syntax_err_msg(err));
    has_err = true;
}

cst_node_t * build_cst(FILE * file) {
    // yydebug = 1;
    has_err = false;
    yyin = file;
    if (yyparse() == 0 && !has_err) {
        return program;
    } else {
        delete_cst_node(program);
        return NULL;
    }
}

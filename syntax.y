%{
int yylex(void);
void yyerror(const char *);
%}

%locations

%union {
    int int_value;
    float float_value;
    const char * string_value;
}

%token <int_value> INT
%token <float_value> FLOAT
%token <string_value> TYPE ID CHAR // LEX_ERR
%token STRUCT IF ELSE WHILE RETURN DOT SEMI COMMA ASSIGN LT LE GT GE NE EQ PLUS MINUS MUL DIV AND OR NOT LP RP LB RB LC RC

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
Program: ExtDefList
    ;
ExtDefList: ExtDef ExtDefList
    | /* empty */
    ;
ExtDef: Specifier ExtDecList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt
    ;
ExtDecList: VarDec
    | VarDec COMMA ExtDecList
    ;

/* specifier */
Specifier: TYPE
    | StructSpecifier
    ;
StructSpecifier: STRUCT ID LC DefList RC
    | STRUCT ID
    ;

/* declarator */
VarDec: ID
   | VarDec LB INT RB
   ;
FunDec: ID LP VarList RP
    | ID LP RP
    ;
VarList: ParamDec COMMA VarList
    | ParamDec
    ;
ParamDec: Specifier VarDec
    ;

/* statement */
CompSt: LC DefList StmtList RC
    ;
StmtList: Stmt StmtList
    | /* empty */
    ;
Stmt: Exp SEMI
    | CompSt
    | RETURN Exp SEMI
    | IF LP Exp RP Stmt             %prec LOWER_ELSE
    | IF LP Exp RP Stmt ELSE Stmt
    | WHILE LP Exp RP Stmt
    ;

/* local definition */
DefList: Def DefList
    | /* empty */
    ;
Def: Specifier DecList SEMI
    ;
DecList: Dec
    | Dec COMMA DecList
    ;
Dec: VarDec
    | VarDec ASSIGN Exp
    ;

/* Expression */
Exp: Exp ASSIGN Exp
    | Exp AND Exp
    | Exp OR Exp
    | Exp LT Exp
    | Exp LE Exp
    | Exp GT Exp
    | Exp GE Exp
    | Exp NE Exp
    | Exp EQ Exp
    | Exp PLUS Exp
    | Exp MINUS Exp
    | Exp MUL Exp
    | Exp DIV Exp
    | LP Exp RP
    | MINUS Exp     %prec UMINUS
    | NOT Exp
    | ID LP Args RP
    | ID LP RP
    | Exp LB Exp RB
    | Exp DOT ID    %prec DOT
    | ID
    | INT
    | FLOAT
    | CHAR
    ;

Args: Exp COMMA Args
    | Exp
    ;

%%

void yyerror(const char * msg) {
    fprintf(stderr, "%s\n", msg);
}

int main(int argc, const char ** argv) {
    //yydebug = 1;
    int err = yyparse();
    if (err) {
        puts("Failed!");
    } else {
        puts("Succeed!");
    }
    return 0;
}

#include "lex.yy.c"

%{

#include <cstdio>
#include <list>
#include <string>
#include "ast.hpp"
#include "utils.hpp"
#include "syntax_errs.hpp"

extern "C" int yylex(void);
static void yyerror(const char *);
static void reportSynErr(int, SyntaxErr);
AST::Program * parseFile(FILE *);

extern FILE * yyin;
static AST::Program * program;
static bool hasErr;

%}

%locations

%define api.value.type union

%token <int> INT
%token <double> FLOAT
%token <char> CHAR
%token <std::string *> TYPE
%token <std::string *> ID
%token STRUCT
%token IF ELSE WHILE FOR RETURN SEMI
%token DOT COMMA ASSIGN LT LE GT GE NE EQ PLUS MINUS MUL DIV AND OR NOT
%token LP RP LB RB LC RC
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

%type <AST::Program*> Program
%type <std::list<AST::ExtDef*>*> ExtDefList
%type <AST::ExtDef*> ExtDef
%type <std::list<AST::VarDec*>*> ExtDecList
%type <AST::Specifier*> Specifier
%type <AST::Specifier*> StructSpecifier
%type <AST::VarDec*> VarDec
%type <AST::FunDec*> FunDec
%type <std::list<AST::ParamDec*>*> VarList
%type <AST::ParamDec*> ParamDec
%type <AST::CompoundStmt*> CompSt
%type <std::list<AST::Stmt*>*> StmtList
%type <AST::Stmt*> Stmt
%type <std::list<AST::Def*>*> DefList
%type <AST::Def*> Def
%type <std::list<AST::Dec*>*> DecList
%type <AST::Dec*> Dec
%type <AST::Exp*> Exp
%type <std::list<AST::Exp*>*> Args

%%
/* high-level definition */
Program:
    ExtDefList {
        program = $$ = new AST::Program(*$1);
        delete $1;
        $$->setLocation(&@$);
        }
    ;
ExtDefList:
    ExtDef ExtDefList {
        $2->push_front($1);
        $$ = $2;
        }
    | /* empty */ {
        $$ = new std::list<AST::ExtDef*>;
        }
    | LEX_ERR_BLK {
        $$ = new std::list<AST::ExtDef*>;
        hasErr = true;
        }
    ;
ExtDef:
    Specifier ExtDecList SEMI {
        $$ = new AST::ExtVarDef($1, *$2);
        delete $2;
        $$->setLocation(&@$);
        }
    | Specifier SEMI {
        $$ = new AST::StructDef($1);
        $$->setLocation(&@$);
        }
    | Specifier FunDec CompSt {
        $$ = new AST::FunDef($1, $2, $3);
        $$->setLocation(&@$);
        }
    | Specifier ExtDecList %prec ERROR {
        $$ = new AST::ExtVarDef($1, *$2);
        delete $2;
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_SEMI);
        }
    | Specifier %prec ERROR {
        $$ = new AST::StructDef($1);
        reportSynErr(@$.last_line, SYNTAX_ERR_MISSING_SEMI);
        }
    ;
ExtDecList:
    VarDec {
        $$ = new std::list<AST::VarDec*>{ $1 };
        }
    | VarDec COMMA ExtDecList {
        $3->push_front($1);
        $$ = $3;
        }
    ;

/* specifier */
Specifier:
    TYPE {
        $$ = new AST::PrimitiveSpecifier(*$1);
        delete $1;
        $$->setLocation(&@$);
        }
    | StructSpecifier {
        $$ = $1;
        $$->setLocation(&@$);
        }
    ;
StructSpecifier:
    STRUCT ID LC DefList RC {
        $$ = new AST::StructSpecifier(*$2, *$4);
        deleteAll($2, $4);
        $$->setLocation(&@$);
        }
    | STRUCT ID {
        $$ = new AST::StructSpecifier(*$2);
        delete $2;
        $$->setLocation(&@$);
        }
    ;

/* declarator */
VarDec:
    ID {
        $$ = new AST::VarDec(*$1);
        delete $1;
        $$->setLocation(&@$);
        }
    | VarDec LB INT RB {
        $$ = new AST::ArrDec(*$1, $3);
        delete $1;
        $$->setLocation(&@$);
        }
    | LEX_ERR %prec ERROR {
        $$ = new AST::VarDec("#err");
        hasErr = true;
        }
    | VarDec LB INT %prec ERROR {
        $$ = new AST::ArrDec(*$1, $3);
        delete $1;
        reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RB);
        }
    ;
FunDec:
    ID LP VarList RP {
        $$ = new AST::FunDec(*$1, *$3);
        deleteAll($1, $3);
        $$->setLocation(&@$);
        }
    | ID LP RP {
        $$ = new AST::FunDec(*$1);
        delete $1;
        $$->setLocation(&@$);
        }
    | ID LP VarList %prec ERROR {
        $$ = new AST::FunDec(*$1, *$3);
        deleteAll($1, $3);
        reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RP);
        }
    | ID LP %prec ERROR {
        $$ = new AST::FunDec(*$1);
        delete $1;
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_RP);
        }
    | ID RP %prec ERROR {
        $$ = new AST::FunDec(*$1);
        delete $1;
        reportSynErr(@2.first_line, SYNTAX_ERR_MISSING_LP);
        }
    ;
VarList:
    ParamDec COMMA VarList {
        $3->push_front($1);
        $$ = $3;
        }
    | ParamDec {
        $$ = new std::list<AST::ParamDec*>{ $1 };
        }
    ;
ParamDec:
    Specifier VarDec {
        $$ = new AST::ParamDec($1, $2);
        $$->setLocation(&@$);
        }
    ;

/* statement */
CompSt:
    LC DefList StmtList RC {
        $$ = new AST::CompoundStmt(*$2, *$3);
        deleteAll($2, $3);
        $$->setLocation(&@$);
        }
    ;
StmtList:
    Stmt StmtList {
        $2->push_front($1);
        $$ = $2;
        }
    | /* empty */ {
        $$ = new std::list<AST::Stmt*>();
        }
    | Stmt Def StmtList %prec ERROR {
        $3->push_front($1);
        $$ = $3;
        delete $2;
        reportSynErr(@1.last_line, SYNTAX_ERR_DEC_STMT_ORDER);
        }
    ;
Stmt:
    Exp SEMI {
        $$ = new AST::ExpStmt($1);
        $$->setLocation(&@$);
        }
    | CompSt {
        $$ = $1;
        $$->setLocation(&@$);
        }
    | RETURN Exp SEMI {
        $$ = new AST::ReturnStmt($2);
        $$->setLocation(&@$);
        }
    | IF LP Exp RP Stmt %prec LOWER_ELSE {
        $$ = new AST::IfStmt($3, $5);
        $$->setLocation(&@$);
        }
    | IF LP Exp RP Stmt ELSE Stmt {
        $$ = new AST::IfStmt($3, $5, $7);
        $$->setLocation(&@$);
        }
    | WHILE LP Exp RP Stmt {
        $$ = new AST::WhileStmt($3, $5);
        $$->setLocation(&@$);
        }
    | FOR LP SEMI SEMI RP Stmt {
        $$ = new AST::ForStmt(nullptr, nullptr, nullptr, $6);
        $$->setLocation(&@$);
        }
    | FOR LP Exp SEMI SEMI RP Stmt {
        $$ = new AST::ForStmt($3, nullptr, nullptr, $7);
        $$->setLocation(&@$);
        }
    | FOR LP SEMI Exp SEMI RP Stmt {
        $$ = new AST::ForStmt(nullptr, $4, nullptr, $7);
        $$->setLocation(&@$);
        }
    | FOR LP SEMI SEMI Exp RP Stmt {
        $$ = new AST::ForStmt(nullptr, nullptr, $5, $7);
        $$->setLocation(&@$);
        }
    | FOR LP Exp SEMI Exp SEMI RP Stmt {
        $$ = new AST::ForStmt($3, $5, nullptr, $8);
        $$->setLocation(&@$);
        }
    | FOR LP Exp SEMI SEMI Exp RP Stmt {
        $$ = new AST::ForStmt($3, nullptr, $6, $8);
        $$->setLocation(&@$);
        }
    | FOR LP SEMI Exp SEMI Exp RP Stmt {
        $$ = new AST::ForStmt(nullptr, $4, $6, $8);
        $$->setLocation(&@$);
        }
    | FOR LP Exp SEMI Exp SEMI Exp RP Stmt {
        $$ = new AST::ForStmt($3, $5, $7, $9);
        $$->setLocation(&@$);
        }
    | Exp %prec ERROR {
        $$ = new AST::ExpStmt($1);
        reportSynErr(@$.last_line, SYNTAX_ERR_MISSING_SEMI);
        }
    | RETURN Exp %prec ERROR {
        $$ = new AST::ReturnStmt($2);
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_SEMI);
        }
    | LEX_ERR_BLK %prec ERROR {
        $$ = new AST::ReturnStmt();
        hasErr = true;
        }
    | error SEMI %prec ERROR {
        $$ = new AST::ReturnStmt();
        }
    ;

/* local definition */
DefList:
    Def DefList {
        $2->push_front($1);
        $$ = $2;
        }
    | /* empty */ {
        $$ = new std::list<AST::Def*>();
        }
    ;
Def:
    Specifier DecList SEMI {
        $$ = new AST::Def($1, *$2);
        delete $2;
        $$->setLocation(&@$);
        }
    | Specifier DecList %prec ERROR {
        $$ = new AST::Def($1, *$2);
        delete $2;
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_SEMI);
        }
    ;
DecList:
    Dec {
        $$ = new std::list<AST::Dec*>{ $1 };
        }
    | Dec COMMA DecList {
        $3->push_front($1);
        $$ = $3;
        }
    ;
Dec:
    VarDec {
        $$ = new AST::Dec($1);
        $$->setLocation(&@$);
        }
    | VarDec ASSIGN Exp {
        $$ = new AST::Dec($1, $3);
        $$->setLocation(&@$);
        }
    ;

/* Expression */
Exp:
    Exp ASSIGN Exp {
        $$ = new AST::AssignExp($1, $3);
        $$->setLocation(&@$);
        }
    | Exp AND Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_AND, $3);
        $$->setLocation(&@$);
        }
    | Exp OR Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_OR, $3);
        $$->setLocation(&@$);
        }
    | Exp LT Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_LT, $3);
        $$->setLocation(&@$);
        }
    | Exp LE Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_LE, $3);
        $$->setLocation(&@$);
        }
    | Exp GT Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_GT, $3);
        $$->setLocation(&@$);
        }
    | Exp GE Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_GE, $3);
        $$->setLocation(&@$);
        }
    | Exp NE Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_NE, $3);
        $$->setLocation(&@$);
        }
    | Exp EQ Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_EQ, $3);
        $$->setLocation(&@$);
        }
    | Exp PLUS Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_PLUS, $3);
        $$->setLocation(&@$);
        }
    | Exp MINUS Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_MINUS, $3);
        $$->setLocation(&@$);
        }
    | Exp MUL Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_MUL, $3);
        $$->setLocation(&@$);
        }
    | Exp DIV Exp {
        $$ = new AST::BinaryExp($1, AST::OPT_DIV, $3);
        $$->setLocation(&@$);
        }
    | LP Exp RP {  // FIXME: recover original structure?
        $$ = $2;
        $$->setLocation(&@$);
        }
    | PLUS Exp %prec UPLUS {
        $$ = new AST::UnaryExp(AST::OPT_PLUS, $2);
        $$->setLocation(&@$);
        }
    | MINUS Exp %prec UMINUS {
        $$ = new AST::UnaryExp(AST::OPT_MINUS, $2);
        $$->setLocation(&@$);
        }
    | NOT Exp %prec NOT {
        $$ = new AST::UnaryExp(AST::OPT_NOT, $2);
        $$->setLocation(&@$);
        }
    | ID LP Args RP {
        $$ = new AST::CallExp(*$1, *$3);
        deleteAll($1, $3);
        $$->setLocation(&@$);
        }
    | ID LP RP {
        $$ = new AST::CallExp(*$1);
        delete $1;
        $$->setLocation(&@$);
        }
    | Exp LB Exp RB {
        $$ = new AST::ArrayExp($1, $3);
        $$->setLocation(&@$);
        }
    | Exp DOT ID {
        $$ = new AST::MemberExp($1, *$3);
        delete $3;
        $$->setLocation(&@$);
        }
    | ID {
        $$ = new AST::IdExp(*$1);
        delete $1;
        $$->setLocation(&@$);
        }
    | INT {
        $$ = new AST::LiteralExp($1);
        $$->setLocation(&@$);
        }
    | FLOAT {
        $$ = new AST::LiteralExp($1);
        $$->setLocation(&@$);
        }
    | CHAR {
        $$ = new AST::LiteralExp($1);
        $$->setLocation(&@$);
        }
    | LEX_ERR {
        $$ = new AST::Exp;
        hasErr = true;
        }
    | Exp LEX_ERR Exp  %prec ERROR {
        deleteAll($1, $3);
        $$ = new AST::Exp;
        hasErr = true;
        }
    | LP Exp %prec ERROR {
        $$ = $2;
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_RP);
        }
    | ID LP Args %prec ERROR {
        $$ = new AST::CallExp(*$1, *$3);
        deleteAll($1, $3);
        reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RP);
        }
    | ID LP %prec ERROR {
        $$ = new AST::CallExp(*$1);
        delete $1;
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_RP);
        }
    | Exp LB Exp %prec ERROR {
        $$ = new AST::ArrayExp($1, $3);
        reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RB);
        }
    ;

Args:
    Exp COMMA Args %prec ARGS {
        $3->push_front($1);
        $$ = $3;
        }
    | Exp %prec ARGS {
        $$ = new std::list<AST::Exp*>{ $1 };
        }
    ;
%%

static void yyerror(const char * msg) {
    // fprintf(stderr, "Error type B at Line %d: %s\n", yylloc.last_line, msg);
    // FIXME: lineno
    fprintf(stderr, "Error type B at Line #: %s\n", msg);
    hasErr = true;
}

static void reportSynErr(int lineno, SyntaxErr err) {
    fprintf(stderr, "Error type B at Line %d: %s\n", lineno, syntaxErrMsgs[err]);
    hasErr = true;
}

AST::Program * parseFile(FILE * file) {
    // yydebug = 1;
    hasErr = false;
    yyin = file;
    if (yyparse() == 0 && !hasErr) {
        return program;
    } else {
        delete program;
        program = nullptr;
        return nullptr;
    }
}

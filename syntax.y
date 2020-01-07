%{

#include <cstdio>
#include <list>
#include <memory>
#include <string>
#include "ast.hpp"
#include "utils.hpp"
#include "syntax_errs.hpp"

extern "C" int yylex(void);
static void yyerror(const char *);
static void reportSynErr(int, SyntaxErr);
ast::Program * parseFile(FILE *);

extern FILE * yyin;
static ast::Program * program;
static bool hasErr;

struct yy_buffer_state;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

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

%type <ast::Program*> Program
%type <std::list<ast::ExtDef*>*> ExtDefList
%type <ast::ExtDef*> ExtDef
%type <std::list<ast::VarDec*>*> ExtDecList
%type <ast::Specifier*> Specifier
%type <ast::Specifier*> StructSpecifier
%type <ast::VarDec*> VarDec
%type <ast::FunDec*> FunDec
%type <std::list<ast::ParamDec*>*> VarList
%type <ast::ParamDec*> ParamDec
%type <ast::CompoundStmt*> CompSt
%type <std::list<ast::Stmt*>*> StmtList
%type <ast::Stmt*> Stmt
%type <std::list<ast::Def*>*> DefList
%type <ast::Def*> Def
%type <std::list<ast::Dec*>*> DecList
%type <ast::Dec*> Dec
%type <ast::Exp*> Exp
%type <std::list<ast::Exp*>*> Args

%%
/* high-level definition */
Program:
    ExtDefList {
        program = $$ = new ast::Program(*$1);
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
        $$ = new std::list<ast::ExtDef*>;
        }
    | LEX_ERR_BLK {
        $$ = new std::list<ast::ExtDef*>;
        hasErr = true;
        }
    ;
ExtDef:
    Specifier ExtDecList SEMI {
        $$ = new ast::ExtVarDef($1, *$2);
        delete $2;
        $$->setLocation(&@$);
        }
    | Specifier SEMI {
        $$ = new ast::StructDef($1);
        $$->setLocation(&@$);
        }
    | Specifier FunDec CompSt {
        $$ = new ast::FunDef($1, $2, $3);
        $$->setLocation(&@$);
        }
    | Specifier ExtDecList %prec ERROR {
        $$ = new ast::ExtVarDef($1, *$2);
        delete $2;
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_SEMI);
        }
    | Specifier %prec ERROR {
        $$ = new ast::StructDef($1);
        reportSynErr(@$.last_line, SYNTAX_ERR_MISSING_SEMI);
        }
    ;
ExtDecList:
    VarDec {
        $$ = new std::list<ast::VarDec*>{ $1 };
        }
    | VarDec COMMA ExtDecList {
        $3->push_front($1);
        $$ = $3;
        }
    ;

/* specifier */
Specifier:
    TYPE {
        $$ = new ast::PrimitiveSpecifier(*$1);
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
        $$ = new ast::StructSpecifier(*$2, *$4);
        deleteAll($2, $4);
        $$->setLocation(&@$);
        }
    | STRUCT ID {
        $$ = new ast::StructSpecifier(*$2);
        delete $2;
        $$->setLocation(&@$);
        }
    ;

/* declarator */
VarDec:
    ID {
        $$ = new ast::VarDec(*$1);
        delete $1;
        $$->setLocation(&@$);
        }
    | VarDec LB INT RB {
        $$ = new ast::ArrDec(*$1, $3);
        delete $1;
        $$->setLocation(&@$);
        }
    | LEX_ERR %prec ERROR {
        $$ = new ast::VarDec("#err");
        hasErr = true;
        }
    | VarDec LB INT %prec ERROR {
        $$ = new ast::ArrDec(*$1, $3);
        delete $1;
        reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RB);
        }
    ;
FunDec:
    ID LP VarList RP {
        $$ = new ast::FunDec(*$1, *$3);
        deleteAll($1, $3);
        $$->setLocation(&@$);
        }
    | ID LP RP {
        $$ = new ast::FunDec(*$1);
        delete $1;
        $$->setLocation(&@$);
        }
    | ID LP VarList %prec ERROR {
        $$ = new ast::FunDec(*$1, *$3);
        deleteAll($1, $3);
        reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RP);
        }
    | ID LP %prec ERROR {
        $$ = new ast::FunDec(*$1);
        delete $1;
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_RP);
        }
    | ID RP %prec ERROR {
        $$ = new ast::FunDec(*$1);
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
        $$ = new std::list<ast::ParamDec*>{ $1 };
        }
    ;
ParamDec:
    Specifier VarDec {
        $$ = new ast::ParamDec($1, $2);
        $$->setLocation(&@$);
        }
    ;

/* statement */
CompSt:
    LC DefList StmtList RC {
        $$ = new ast::CompoundStmt(*$2, *$3);
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
        $$ = new std::list<ast::Stmt*>();
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
        $$ = new ast::ExpStmt($1);
        $$->setLocation(&@$);
        }
    | CompSt {
        $$ = $1;
        $$->setLocation(&@$);
        }
    | RETURN Exp SEMI {
        $$ = new ast::ReturnStmt($2);
        $$->setLocation(&@$);
        }
    | IF LP Exp RP Stmt %prec LOWER_ELSE {
        $$ = new ast::IfStmt($3, $5);
        $$->setLocation(&@$);
        }
    | IF LP Exp RP Stmt ELSE Stmt {
        $$ = new ast::IfStmt($3, $5, $7);
        $$->setLocation(&@$);
        }
    | WHILE LP Exp RP Stmt {
        $$ = new ast::WhileStmt($3, $5);
        $$->setLocation(&@$);
        }
    | FOR LP SEMI SEMI RP Stmt {
        $$ = new ast::ForStmt(nullptr, nullptr, nullptr, $6);
        $$->setLocation(&@$);
        }
    | FOR LP Exp SEMI SEMI RP Stmt {
        $$ = new ast::ForStmt($3, nullptr, nullptr, $7);
        $$->setLocation(&@$);
        }
    | FOR LP SEMI Exp SEMI RP Stmt {
        $$ = new ast::ForStmt(nullptr, $4, nullptr, $7);
        $$->setLocation(&@$);
        }
    | FOR LP SEMI SEMI Exp RP Stmt {
        $$ = new ast::ForStmt(nullptr, nullptr, $5, $7);
        $$->setLocation(&@$);
        }
    | FOR LP Exp SEMI Exp SEMI RP Stmt {
        $$ = new ast::ForStmt($3, $5, nullptr, $8);
        $$->setLocation(&@$);
        }
    | FOR LP Exp SEMI SEMI Exp RP Stmt {
        $$ = new ast::ForStmt($3, nullptr, $6, $8);
        $$->setLocation(&@$);
        }
    | FOR LP SEMI Exp SEMI Exp RP Stmt {
        $$ = new ast::ForStmt(nullptr, $4, $6, $8);
        $$->setLocation(&@$);
        }
    | FOR LP Exp SEMI Exp SEMI Exp RP Stmt {
        $$ = new ast::ForStmt($3, $5, $7, $9);
        $$->setLocation(&@$);
        }
    | Exp %prec ERROR {
        $$ = new ast::ExpStmt($1);
        reportSynErr(@$.last_line, SYNTAX_ERR_MISSING_SEMI);
        }
    | RETURN Exp %prec ERROR {
        $$ = new ast::ReturnStmt($2);
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_SEMI);
        }
    | LEX_ERR_BLK %prec ERROR {
        $$ = new ast::ReturnStmt();
        hasErr = true;
        }
    | error SEMI %prec ERROR {
        $$ = new ast::ReturnStmt();
        }
    ;

/* local definition */
DefList:
    Def DefList {
        $2->push_front($1);
        $$ = $2;
        }
    | /* empty */ {
        $$ = new std::list<ast::Def*>();
        }
    ;
Def:
    Specifier DecList SEMI {
        $$ = new ast::Def($1, *$2);
        delete $2;
        $$->setLocation(&@$);
        }
    | Specifier DecList %prec ERROR {
        $$ = new ast::Def($1, *$2);
        delete $2;
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_SEMI);
        }
    ;
DecList:
    Dec {
        $$ = new std::list<ast::Dec*>{ $1 };
        }
    | Dec COMMA DecList {
        $3->push_front($1);
        $$ = $3;
        }
    ;
Dec:
    VarDec {
        $$ = new ast::Dec($1);
        $$->setLocation(&@$);
        }
    | VarDec ASSIGN Exp {
        $$ = new ast::Dec($1, $3);
        $$->setLocation(&@$);
        }
    ;

/* Expression */
Exp:
    Exp ASSIGN Exp {
        $$ = new ast::AssignExp($1, $3);
        $$->setLocation(&@$);
        }
    | Exp AND Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_AND, $3);
        $$->setLocation(&@$);
        }
    | Exp OR Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_OR, $3);
        $$->setLocation(&@$);
        }
    | Exp LT Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_LT, $3);
        $$->setLocation(&@$);
        }
    | Exp LE Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_LE, $3);
        $$->setLocation(&@$);
        }
    | Exp GT Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_GT, $3);
        $$->setLocation(&@$);
        }
    | Exp GE Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_GE, $3);
        $$->setLocation(&@$);
        }
    | Exp NE Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_NE, $3);
        $$->setLocation(&@$);
        }
    | Exp EQ Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_EQ, $3);
        $$->setLocation(&@$);
        }
    | Exp PLUS Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_PLUS, $3);
        $$->setLocation(&@$);
        }
    | Exp MINUS Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_MINUS, $3);
        $$->setLocation(&@$);
        }
    | Exp MUL Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_MUL, $3);
        $$->setLocation(&@$);
        }
    | Exp DIV Exp {
        $$ = new ast::BinaryExp($1, ast::OPT_DIV, $3);
        $$->setLocation(&@$);
        }
    | LP Exp RP {  // FIXME: recover original structure?
        $$ = $2;
        $$->setLocation(&@$);
        }
    | PLUS Exp %prec UPLUS {
        $$ = new ast::UnaryExp(ast::OPT_PLUS, $2);
        $$->setLocation(&@$);
        }
    | MINUS Exp %prec UMINUS {
        $$ = new ast::UnaryExp(ast::OPT_MINUS, $2);
        $$->setLocation(&@$);
        }
    | NOT Exp %prec NOT {
        $$ = new ast::UnaryExp(ast::OPT_NOT, $2);
        $$->setLocation(&@$);
        }
    | ID LP Args RP {
        $$ = new ast::CallExp(*$1, *$3);
        deleteAll($1, $3);
        $$->setLocation(&@$);
        }
    | ID LP RP {
        $$ = new ast::CallExp(*$1);
        delete $1;
        $$->setLocation(&@$);
        }
    | Exp LB Exp RB {
        $$ = new ast::ArrayExp($1, $3);
        $$->setLocation(&@$);
        }
    | Exp DOT ID {
        $$ = new ast::MemberExp($1, *$3);
        delete $3;
        $$->setLocation(&@$);
        }
    | ID {
        $$ = new ast::IdExp(*$1);
        delete $1;
        $$->setLocation(&@$);
        }
    | INT {
        $$ = new ast::LiteralExp($1);
        $$->setLocation(&@$);
        }
    | FLOAT {
        $$ = new ast::LiteralExp($1);
        $$->setLocation(&@$);
        }
    | CHAR {
        $$ = new ast::LiteralExp($1);
        $$->setLocation(&@$);
        }
    | LEX_ERR {
        $$ = new ast::IdExp("__lex_err__");
        hasErr = true;
        }
    | Exp LEX_ERR Exp  %prec ERROR {
        deleteAll($1, $3);
        $$ = new ast::IdExp("__lex_err__");
        hasErr = true;
        }
    | LP Exp %prec ERROR {
        $$ = $2;
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_RP);
        }
    | ID LP Args %prec ERROR {
        $$ = new ast::CallExp(*$1, *$3);
        deleteAll($1, $3);
        reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RP);
        }
    | ID LP %prec ERROR {
        $$ = new ast::CallExp(*$1);
        delete $1;
        reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_RP);
        }
    | Exp LB Exp %prec ERROR {
        $$ = new ast::ArrayExp($1, $3);
        reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RB);
        }
    ;

Args:
    Exp COMMA Args %prec ARGS {
        $3->push_front($1);
        $$ = $3;
        }
    | Exp %prec ARGS {
        $$ = new std::list<ast::Exp*>{ $1 };
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

ast::Program * parseFile(FILE * file) {
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

ast::Program * parseStr(const char * src) {
    hasErr = false;
    YY_BUFFER_STATE buffer = yy_scan_string(src);
    if (yyparse() != 0 || hasErr) {
        delete program;
        program = nullptr;
    }
    yy_delete_buffer(buffer);
    return program;
}

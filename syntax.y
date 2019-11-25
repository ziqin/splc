%{
    #include <cstdio>
    #include "cst.hpp"
    #include "parser.hpp"
    #include "syntax_errs.hpp"

    extern "C" int yylex(void);
    static void yyerror(const char *);
    static void reportSynErr(int, SyntaxErr);

    extern FILE * yyin;
    static CST::Node * program;
    static bool hasErr;
%}

%locations

%define api.value.type { CST::Node * }

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
Program: ExtDefList                 { program = $$ = new CST::NtNode(CST::Program, &@$, { $1 }); }
    ;
ExtDefList: ExtDef ExtDefList       { $$ = new CST::NtNode(CST::ExtDefList, &@$, { $1, $2 }); }
    | /* empty */                   { $$ = new CST::NtNode(CST::ExtDefList, &@$, {}); }
    | LEX_ERR_BLK                   { $$ = nullptr; hasErr = true; }
    ;
ExtDef: Specifier ExtDecList SEMI            { $$ = new CST::NtNode(CST::ExtDef, &@$, { $1, $2, $3 }); }
    | Specifier SEMI                         { $$ = new CST::NtNode(CST::ExtDef, &@$, { $1, $2 }); }
    | Specifier FunDec CompSt                { $$ = new CST::NtNode(CST::ExtDef, &@$, { $1, $2, $3 }); }
    | Specifier ExtDecList       %prec ERROR { $$ = nullptr; reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_SEMI); }
    | Specifier                  %prec ERROR { $$ = nullptr; reportSynErr(@$.last_line, SYNTAX_ERR_MISSING_SEMI); }
    ;
ExtDecList: VarDec               { $$ = new CST::NtNode(CST::ExtDecList, &@$, { $1 }); }
    | VarDec COMMA ExtDecList    { $$ = new CST::NtNode(CST::ExtDecList, &@$, { $1, $2, $3} ); }
    ;

/* specifier */
Specifier: TYPE                             { $$ = new CST::NtNode(CST::Specifier, &@$, { $1 }); }
    | StructSpecifier                       { $$ = new CST::NtNode(CST::Specifier, &@$, { $1 }); }
    ;
StructSpecifier: STRUCT ID LC DefList RC    { $$ = new CST::NtNode(CST::StructSpecifier, &@$, { $1, $2, $3, $4, $5 }); }
    | STRUCT ID                             { $$ = new CST::NtNode(CST::StructSpecifier, &@$, { $1, $2 }); }
    // | STRUCT ID LC DefList      %prec ERROR { $$ = nullptr; delete $1; delete $2; delete $3; delete $4; reportSynErr(@4.last_line, SYNTAX_ERR_MISSING_RC); }
    ;

/* declarator */
VarDec: ID                      { $$ = new CST::NtNode(CST::VarDec, &@$, { $1 }); }
    | VarDec LB INT RB          { $$ = new CST::NtNode(CST::VarDec, &@$, { $1, $2, $3, $4 }); }
    | LEX_ERR       %prec ERROR { $$ = nullptr; hasErr = true; }
    | VarDec LB INT %prec ERROR { $$ = nullptr; delete $1; delete $2; delete $3; reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RB); }
    ;
FunDec: ID LP VarList RP        { $$ = new CST::NtNode(CST::FunDec, &@$, { $1, $2, $3, $4 }); }
    | ID LP RP                  { $$ = new CST::NtNode(CST::FunDec, &@$, { $1, $2, $3 }); }
    | ID LP VarList %prec ERROR { $$ = nullptr; delete $1; delete $2; delete $3; reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RP); }
    | ID LP         %prec ERROR { $$ = nullptr; delete $1; delete $2; reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_RP); }
    // | ID VarList RP %prec ERROR { $$ = nullptr; delete $1; delete $2; delete $3; reportSynErr(@2.first_line, SYNTAX_ERR_MISSING_LP); }
    | ID RP         %prec ERROR { $$ = nullptr; delete $1; delete $2; reportSynErr(@2.first_line, SYNTAX_ERR_MISSING_LP); }
    ;
VarList: ParamDec COMMA VarList { $$ = new CST::NtNode(CST::VarList, &@$, { $1, $2, $3 }); }
    | ParamDec                  { $$ = new CST::NtNode(CST::VarList, &@$, { $1 }); }
    ;
ParamDec: Specifier VarDec      { $$ = new CST::NtNode(CST::ParamDec, &@$, { $1, $2 }); }
    ;

/* statement */
CompSt: LC DefList StmtList RC              { $$ = new CST::NtNode(CST::CompSt, &@$, { $1, $2, $3, $4 }); }
    // | LC DefList StmtList       %prec ERROR { $$ = nullptr; delete $1; delete $2; delete $3; reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RC); }
    ;
StmtList: Stmt StmtList                     { $$ = new CST::NtNode(CST::StmtList, &@$, { $1, $2 }); }
    | /* empty */                           { $$ = new CST::NtNode(CST::StmtList, &@$, {}); }
    | Stmt Def StmtList         %prec ERROR { $$ = nullptr; delete $1; delete $2; reportSynErr(@1.last_line, SYNTAX_ERR_DEC_STMT_ORDER); }
    ;
Stmt: Exp SEMI                              { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2 }); }
    | CompSt                                { $$ = new CST::NtNode(CST::Stmt, &@$, { $1 }); }
    | RETURN Exp SEMI                       { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, $3 }); }
    | IF LP Exp RP Stmt %prec LOWER_ELSE    { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, $3, $4, $5 }); }
    | IF LP Exp RP Stmt ELSE Stmt           { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, $3, $4, $5, $6, $7 }); }
    | WHILE LP Exp RP Stmt                  { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, $3, $4, $5 }); }
    | FOR LP SEMI SEMI RP Stmt              { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, nullptr, $3, nullptr, $4, nullptr, $5, $6 }); }
    | FOR LP Exp SEMI SEMI RP Stmt          { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, $3, $4, nullptr, $5, nullptr, $6, $7 }); }
    | FOR LP SEMI Exp SEMI RP Stmt          { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, nullptr, $3, $4, $5, nullptr, $6, $7 }); }
    | FOR LP SEMI SEMI Exp RP Stmt          { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, nullptr, $3, nullptr, $4, $5, $6, $7 }); }
    | FOR LP Exp SEMI Exp SEMI RP Stmt      { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, $3, $4, $5, $6, nullptr, $7, $8 }); }
    | FOR LP Exp SEMI SEMI Exp RP Stmt      { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, $3, $4, nullptr, $5, $6, $7, $8 }); }
    | FOR LP SEMI Exp SEMI Exp RP Stmt      { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, nullptr, $3, $4, $5, $6, $7, $8 }); }
    | FOR LP Exp SEMI Exp SEMI Exp RP Stmt  { $$ = new CST::NtNode(CST::Stmt, &@$, { $1, $2, $3, $4, $5, $6, $7, $8, $9 }); }
    | Exp                       %prec ERROR { $$ = nullptr; delete $1; reportSynErr(@$.last_line, SYNTAX_ERR_MISSING_SEMI); }
    | RETURN Exp                %prec ERROR { $$ = nullptr; delete $1; delete $2; reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_SEMI); }
    | LEX_ERR_BLK               %prec ERROR { $$ = nullptr; hasErr = true; }
    | error SEMI                %prec ERROR { $$ = nullptr; delete $2; }
    // TODO:
    // | IF LP Exp Stmt      %prec LOWER_ERROR { $$ = nullptr; reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RP); }
    // | IF LP Exp Stmt ELSE Stmt  %prec ERROR { $$ = nullptr; reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RP); }
    // | WHILE LP Exp Stmt         %prec ERROR { $$ = nullptr; reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RP); }
    ;

/* local definition */
DefList: Def DefList            { $$ = new CST::NtNode(CST::DefList, &@$, { $1, $2 }); }
    | /* empty */               { $$ = new CST::NtNode(CST::DefList, &@$, {}); }
    ;
Def: Specifier DecList SEMI               { $$ = new CST::NtNode(CST::Def, &@$, { $1, $2, $3 }); }
    | Specifier DecList       %prec ERROR { $$ = nullptr; delete $1; delete $2; 
                                            reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_SEMI); }
    ;
DecList: Dec                    { $$ = new CST::NtNode(CST::DecList, &@$, { $1 }); }
    | Dec COMMA DecList         { $$ = new CST::NtNode(CST::DecList, &@$, { $1, $2, $3 }); }
    ;
Dec: VarDec                     { $$ = new CST::NtNode(CST::Dec, &@$, { $1 }); }
    | VarDec ASSIGN Exp         { $$ = new CST::NtNode(CST::Dec, &@$, { $1, $2, $3 }); }
    ;

/* Expression */
Exp: Exp ASSIGN Exp                 { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp AND Exp                   { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp OR Exp                    { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp LT Exp                    { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp LE Exp                    { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp GT Exp                    { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp GE Exp                    { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp NE Exp                    { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp EQ Exp                    { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp PLUS Exp                  { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp MINUS Exp                 { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp MUL Exp                   { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp DIV Exp                   { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | LP Exp RP                     { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | PLUS Exp         %prec UPLUS  { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2 }); }
    | MINUS Exp        %prec UMINUS { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2 }); }
    | NOT Exp          %prec NOT    { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2 }); }
    | ID LP Args RP                 { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3, $4 }); }
    | ID LP RP                      { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | Exp LB Exp RB                 { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3, $4 }); }
    | Exp DOT ID                    { $$ = new CST::NtNode(CST::Exp, &@$, { $1, $2, $3 }); }
    | ID                            { $$ = new CST::NtNode(CST::Exp, &@$, { $1 }); }
    | INT                           { $$ = new CST::NtNode(CST::Exp, &@$, { $1 }); }
    | FLOAT                         { $$ = new CST::NtNode(CST::Exp, &@$, { $1 }); }
    | CHAR                          { $$ = new CST::NtNode(CST::Exp, &@$, { $1 }); }
    | LEX_ERR                       { $$ = nullptr; hasErr = true; }
    | Exp LEX_ERR Exp  %prec ERROR  { $$ = nullptr; hasErr = true; }
    | LP Exp           %prec ERROR  { $$ = nullptr; delete $1; delete $2; reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_RP); }
    | ID LP Args       %prec ERROR  { $$ = nullptr; delete $1; delete $2; delete $3; reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RP); }
    | ID LP            %prec ERROR  { $$ = nullptr; delete $1; delete $2; reportSynErr(@2.last_line, SYNTAX_ERR_MISSING_RP); }
    | Exp LB Exp       %prec ERROR  { $$ = nullptr; delete $1; delete $2; delete $3; reportSynErr(@3.last_line, SYNTAX_ERR_MISSING_RB); }
    // TODO:
    // | Exp RP           %prec ERROR  { $$ = nullptr; delete $1; delete $2; reportSynErr(@1.first_line, SYNTAX_ERR_MISSING_LP); }
    // | ID Args RP       %prec ERROR  { $$ = nullptr; delete $1; delete $2; delete $3; reportSynErr(@1.last_line, SYNTAX_ERR_MISSING_LP); }
    // | ID RP            %prec ERROR  { $$ = nullptr; delete $1; delete $2; reportSynErr(@1.last_line, SYNTAX_ERR_MISSING_LP); }
    ;

Args: Exp COMMA Args    %prec ARGS { $$ = new CST::NtNode(CST::Args, &@$, { $1, $2, $3 }); }
    | Exp               %prec ARGS { $$ = new CST::NtNode(CST::Args, &@$, { $1 }); }
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

CST::Node * parseFile(FILE * file) {
    // yydebug = 1;
    hasErr = false;
    yyin = file;
    if (yyparse() == 0 && !hasErr) {
        return program;
    } else {
        delete program;
        return nullptr;
    }
}

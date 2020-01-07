#ifndef AST_HPP
#define AST_HPP

#include <initializer_list>
#include <list>
#include <memory>
#include <string>
#include <vector>
#include "symbol_table.hpp"
#include "tac.hpp"
#include "type.hpp"
#include "utils.hpp"


struct YYLTYPE;

namespace ast {

#define FOR_EACH_NODE(action)   \
    action(Node)                \
    action(Program)             \
    action(VarDec)              \
    action(ArrDec)              \
    action(Dec)                 \
    action(Def)                 \
    action(ParamDec)            \
    action(FunDec)              \
    action(Specifier)           \
    action(PrimitiveSpecifier)  \
    action(StructSpecifier)     \
    action(ExtDef)              \
    action(ExtVarDef)           \
    action(StructDef)           \
    action(FunDef)              \
    action(Exp)                 \
    action(LiteralExp)          \
    action(IdExp)               \
    action(ArrayExp)            \
    action(MemberExp)           \
    action(UnaryExp)            \
    action(BinaryExp)           \
    action(AssignExp)           \
    action(CallExp)             \
    action(Stmt)                \
    action(ExpStmt)             \
    action(ReturnStmt)          \
    action(IfStmt)              \
    action(WhileStmt)           \
    action(ForStmt)             \
    action(CompoundStmt)


class Visitor;

#define DECLARE_STRUCT(T) struct T;
FOR_EACH_NODE(DECLARE_STRUCT)

struct Location {
    struct Position {
        int line, column;
    } start, end;

    Location() {
        start.line = end.line = 0;
        start.column = end.column = 0;
    }
};

#define DEFINE_VISITOR_HOOKS                                                        \
    virtual void visit(Visitor *visitor);                                           \
    virtual void traverse(std::initializer_list<Visitor*> visitors, Node *parent);

struct Node {
    unsigned nodeId;
    Location loc;
    std::shared_ptr<smt::SymbolTable> scope;
    Node();
    virtual ~Node() = default;
    void setLocation(const YYLTYPE * loc);

    DEFINE_VISITOR_HOOKS
};


struct Program final: public Node {
    std::vector<ExtDef*> extDefs;

    Program(const std::list<ExtDef*>& extDefList);
    ~Program();
    void traverse(std::initializer_list<Visitor*> visitors);

    DEFINE_VISITOR_HOOKS
};


// declaration/definition

struct VarDec: public Node {
    std::string identifier;

    VarDec(const std::string& identifier): identifier(identifier) {}
    virtual ~VarDec() = default;

    DEFINE_VISITOR_HOOKS
};

struct ArrDec final: public VarDec {
    std::vector<int> dimensions;

    ArrDec(const VarDec& declarator, int dim);

    DEFINE_VISITOR_HOOKS
};

struct Dec final: public Node {
    VarDec * declarator;
    Exp * init;

    Dec(VarDec * declarator, Exp * init = nullptr);
    ~Dec();

    DEFINE_VISITOR_HOOKS
};

struct Def: public Node {
    Specifier * specifier;
    std::vector<Dec*> declarations;

    Def(Specifier * specifier, const std::list<Dec*>& decList);
    ~Def();

    DEFINE_VISITOR_HOOKS
};

struct ParamDec: public Node {
    Specifier * specifier;
    VarDec * declarator;

    ParamDec(Specifier * specifier, VarDec * declarator);
    ~ParamDec();

    DEFINE_VISITOR_HOOKS
};

struct FunDec: public Node {
    std::string identifier;
    std::vector<ParamDec*> parameters;

    FunDec(const std::string& identifier, const std::list<ParamDec*>& varList = {});
    ~FunDec();

    DEFINE_VISITOR_HOOKS
};

struct Specifier: public Node {
    Shared<smt::Type> type;

    DEFINE_VISITOR_HOOKS
};

struct PrimitiveSpecifier final: public Specifier {
    smt::Primitive primitive;

    PrimitiveSpecifier(const std::string& typeName);

    DEFINE_VISITOR_HOOKS
};

struct StructSpecifier final: public Specifier {
    std::string identifier;
    std::vector<Def*> definitions;

    StructSpecifier(const std::string& identifier, const std::list<Def*>& defList = {});
    ~StructSpecifier();

    DEFINE_VISITOR_HOOKS
};

struct ExtDef: public Node {
    DEFINE_VISITOR_HOOKS
};

struct ExtVarDef final: public ExtDef {
    Specifier * specifier;
    std::vector<VarDec*> varDecs;

    ExtVarDef(Specifier *specifier, const std::list<VarDec*>& extDecList);
    ~ExtVarDef();

    DEFINE_VISITOR_HOOKS
};

struct StructDef final: public ExtDef {
    StructSpecifier *specifier;

    StructDef(Specifier *specifier);
    ~StructDef();

    DEFINE_VISITOR_HOOKS
};

struct FunDef final: public ExtDef {
    Specifier *specifier;
    FunDec *declarator;
    CompoundStmt *body;

    FunDef(Specifier *specifier, FunDec *declarator, CompoundStmt *body);
    ~FunDef();

    DEFINE_VISITOR_HOOKS
};

// ------------------------ expressions ------------------------------

struct Exp: public Node {
    Shared<smt::Type> type;

    Exp() {}
    Exp(Shared<smt::Type> type): type(type) {}
    virtual ~Exp() = default;

    DEFINE_VISITOR_HOOKS
};


struct LiteralExp final: public Exp {
    union {
        char charVal;
        int intVal;
        double floatVal;
    };

    LiteralExp(char);
    LiteralExp(int);
    LiteralExp(double);

    DEFINE_VISITOR_HOOKS
};


struct IdExp final: public Exp {
    std::string identifier;

    IdExp(const std::string&);

    DEFINE_VISITOR_HOOKS
};

struct ArrayExp: public Exp {
    Exp * subject, * index;

    ArrayExp(Exp * subject, Exp * index);
    ~ArrayExp();

    DEFINE_VISITOR_HOOKS
};

struct MemberExp: public Exp {
    Exp * subject;
    std::string member;

    MemberExp(Exp * subject, const std::string& member);
    ~MemberExp();

    DEFINE_VISITOR_HOOKS
};

enum Operator {
    OPT_AND,
    OPT_OR,
    OPT_LT,
    OPT_LE,
    OPT_GT,
    OPT_GE,
    OPT_NE,
    OPT_EQ,
    OPT_PLUS,
    OPT_MINUS,
    OPT_MUL,
    OPT_DIV,
    OPT_NOT
};

struct UnaryExp: public Exp {
    Operator opt;
    Exp *argument;

    UnaryExp(Operator opt, Exp *argument);
    ~UnaryExp();

    DEFINE_VISITOR_HOOKS
};

struct BinaryExp: public Exp {
    Operator opt;
    Exp *left, *right;

    BinaryExp(Exp *left, Operator opt, Exp *right);
    ~BinaryExp();

    DEFINE_VISITOR_HOOKS
};

struct AssignExp: public Exp {
    Exp *left, *right;

    AssignExp(Exp *left, Exp *right);
    ~AssignExp();

    DEFINE_VISITOR_HOOKS
};

struct CallExp: public Exp {
    std::string identifier;
    std::vector<Exp*> arguments;

    CallExp(const std::string& identifier, const std::list<Exp*>& arguments = {});
    ~CallExp();

    DEFINE_VISITOR_HOOKS
};


// ------------------------------ statements -------------------------------------

struct Stmt: public Node {
    DEFINE_VISITOR_HOOKS
};

struct ExpStmt final: public Stmt {
    Exp *expression;

    ExpStmt(Exp *expression);
    ~ExpStmt() {
        delete expression;
    }

    DEFINE_VISITOR_HOOKS
};

struct ReturnStmt final: public Stmt {
    Exp *argument;

    ReturnStmt(Exp *argument = nullptr): argument(argument) {}
    ~ReturnStmt() {
        delete argument;
    }

    DEFINE_VISITOR_HOOKS
};

struct IfStmt final: public Stmt {
    Exp *test;
    Stmt *consequent, *alternate;

    IfStmt(Exp *test, Stmt *consequent, Stmt *alternate = nullptr);

    DEFINE_VISITOR_HOOKS
};

struct WhileStmt final: public Stmt {
    Exp *test;
    Stmt *body;

    WhileStmt(Exp *test, Stmt *body);

    DEFINE_VISITOR_HOOKS
};

struct ForStmt final: public Stmt {
    Exp *init, *test, *update;
    Stmt *body;

    ForStmt(Exp *init, Exp *test, Exp *update, Stmt *body);

    DEFINE_VISITOR_HOOKS
};

struct CompoundStmt final: public Stmt {
    std::vector<Def*> definitions;
    std::vector<Stmt*> body;

    CompoundStmt(const std::list<Def*>& defList, const std::list<Stmt*>& stmtList):
        definitions(defList.begin(), defList.end()),
        body(stmtList.begin(), stmtList.end()) {}
    ~CompoundStmt() {
        for (auto def: definitions) delete def;
        for (auto stmt: body) delete stmt;
    }

    DEFINE_VISITOR_HOOKS
};

} // namespace ast

#endif // AST_HPP

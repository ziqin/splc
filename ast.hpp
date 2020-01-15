#ifndef AST_HPP
#define AST_HPP

#include <initializer_list>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "symbol_table.hpp"
#include "tac.hpp"
#include "type.hpp"
#include "utils.hpp"


struct YYLTYPE;

namespace ast {

#define FOR_ALL_AST_NODES(action)   \
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
FOR_ALL_AST_NODES(DECLARE_STRUCT)

struct Location {
    struct Position {
        int line, column;
    } start, end;

    Location(): start { .line = 0, .column = 0 }, end {.line = 0, .column = 0} {}
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

    explicit Program(const std::list<ExtDef*>& extDefList);
    ~Program() override;
    void traverse(std::initializer_list<Visitor*> visitors);

    DEFINE_VISITOR_HOOKS
};


// declaration/definition

struct VarDec: public Node {
    std::string identifier;

    explicit VarDec(std::string identifier): identifier(std::move(identifier)) {}
    ~VarDec() override = default;

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

    explicit Dec(VarDec * declarator, Exp * init = nullptr);
    ~Dec() override;

    DEFINE_VISITOR_HOOKS
};

struct Def: public Node {
    Specifier * specifier;
    std::vector<Dec*> declarations;

    Def(Specifier * specifier, const std::list<Dec*>& decList);
    ~Def() override;

    DEFINE_VISITOR_HOOKS
};

struct ParamDec: public Node {
    Specifier * specifier;
    VarDec * declarator;

    ParamDec(Specifier * specifier, VarDec * declarator);
    ~ParamDec() override;

    DEFINE_VISITOR_HOOKS
};

struct FunDec: public Node {
    std::string identifier;
    std::vector<ParamDec*> parameters;

    explicit FunDec(std::string identifier, const std::list<ParamDec*>& varList = {});
    ~FunDec() override;

    DEFINE_VISITOR_HOOKS
};

struct Specifier: public Node {
    Shared<smt::Type> type;

    DEFINE_VISITOR_HOOKS
};

struct PrimitiveSpecifier final: public Specifier {
    smt::Primitive primitive;

    explicit PrimitiveSpecifier(const std::string& typeName);

    DEFINE_VISITOR_HOOKS
};

struct StructSpecifier final: public Specifier {
    std::string identifier;
    std::vector<Def*> definitions;

    explicit StructSpecifier(std::string identifier, const std::list<Def*>& defList = {});
    ~StructSpecifier() override;

    DEFINE_VISITOR_HOOKS
};

struct ExtDef: public Node {
    DEFINE_VISITOR_HOOKS
};

struct ExtVarDef final: public ExtDef {
    Specifier * specifier;
    std::vector<VarDec*> varDecs;

    ExtVarDef(Specifier *specifier, const std::list<VarDec*>& extDecList);
    ~ExtVarDef() override;

    DEFINE_VISITOR_HOOKS
};

struct StructDef final: public ExtDef {
    StructSpecifier *specifier;

    explicit StructDef(Specifier *specifier);
    ~StructDef() override;

    DEFINE_VISITOR_HOOKS
};

struct FunDef final: public ExtDef {
    Specifier *specifier;
    FunDec *declarator;
    CompoundStmt *body;

    FunDef(Specifier *specifier, FunDec *declarator, CompoundStmt *body);
    ~FunDef() override;

    DEFINE_VISITOR_HOOKS
};

// ------------------------ expressions ------------------------------

struct Exp: public Node {
    Shared<smt::Type> type;

    Exp() = default;
    explicit Exp(Shared<smt::Type> type): type(std::move(type)) {}
    ~Exp() override = default;

    DEFINE_VISITOR_HOOKS
};


struct LiteralExp final: public Exp {
    union {
        char charVal;
        int intVal;
        double floatVal;
    };

    explicit LiteralExp(char);
    explicit LiteralExp(int);
    explicit LiteralExp(double);

    DEFINE_VISITOR_HOOKS
};


struct IdExp final: public Exp {
    std::string identifier;

    explicit IdExp(const std::string&);

    DEFINE_VISITOR_HOOKS
};

struct ArrayExp: public Exp {
    Exp *subject, *index;

    ArrayExp(Exp *subject, Exp *index);
    ~ArrayExp() override;

    DEFINE_VISITOR_HOOKS
};

struct MemberExp: public Exp {
    Exp * subject;
    std::string member;

    MemberExp(Exp *subject, std::string member);
    ~MemberExp() override;

    DEFINE_VISITOR_HOOKS
};

enum class Operator {
    AND,
    OR,
    LT,
    LE,
    GT,
    GE,
    NE,
    EQ,
    PLUS,
    MINUS,
    MUL,
    DIV,
    NOT
};

struct UnaryExp: public Exp {
    Operator opt;
    Exp *argument;

    UnaryExp(Operator opt, Exp *argument);
    ~UnaryExp() override;

    DEFINE_VISITOR_HOOKS
};

struct BinaryExp: public Exp {
    Operator opt;
    Exp *left, *right;

    BinaryExp(Exp *left, Operator opt, Exp *right);
    ~BinaryExp() override;

    DEFINE_VISITOR_HOOKS
};

struct AssignExp: public Exp {
    Exp *left, *right;

    AssignExp(Exp *left, Exp *right);
    ~AssignExp() override;

    DEFINE_VISITOR_HOOKS
};

struct CallExp: public Exp {
    std::string identifier;
    std::vector<Exp*> arguments;

    explicit CallExp(std::string identifier, const std::list<Exp*>& arguments = {});
    ~CallExp() override;

    DEFINE_VISITOR_HOOKS
};


// ------------------------------ statements -------------------------------------

struct Stmt: public Node {
    DEFINE_VISITOR_HOOKS
};

struct ExpStmt final: public Stmt {
    Exp *expression;

    explicit ExpStmt(Exp *expression);
    ~ExpStmt() override {
        delete expression;
    }

    DEFINE_VISITOR_HOOKS
};

struct ReturnStmt final: public Stmt {
    Exp *argument;

    explicit ReturnStmt(Exp *argument = nullptr): argument(argument) {}
    ~ReturnStmt() override {
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
    ~CompoundStmt() override {
        for (auto def: definitions) delete def;
        for (auto stmt: body) delete stmt;
    }

    DEFINE_VISITOR_HOOKS
};

} // namespace ast

#endif // AST_HPP

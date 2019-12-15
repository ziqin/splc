#ifndef AST_HPP
#define AST_HPP

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "symbol_table.hpp"
#include "type.hpp"


struct YYLTYPE;

namespace AST {

class Walker;

struct Node;
struct Program;
struct ExtDef;
struct ExtDec;
struct Specifier;
struct StructSpecifier;
struct VarDec;
struct FunDec;
struct ParamDec;
struct Stmt;
struct CompoundStmt;
struct Def;
struct Dec;
struct Exp;

struct Location {
    struct Position {
        int line, column;
    } start, end;

    Location() {
        start.line = end.line = 0;
        start.column = end.column = 0;
    }
};

struct Node {
    unsigned id;
    Location loc;
    std::shared_ptr<SymbolTable> scope;
    Node();
    virtual ~Node() {}
    virtual void setScope(std::shared_ptr<SymbolTable>);
    virtual void traverse(const std::vector<Walker*>& walkers, Node * parent);
    void setLocation(const YYLTYPE * loc);
};


// misc

struct VarDec: public Node {
    std::string identifier;

    VarDec(const std::string& id): identifier(id) {}
};

struct ArrDec final: public VarDec {
    std::vector<int> dimensions;

    ArrDec(const VarDec& declarator, int dim);
};

struct Dec final: public Node {
    VarDec * declarator;
    Exp * init;

    Dec(VarDec * declarator, Exp * init = nullptr);
    ~Dec();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct Def: public Node {
    Specifier * specifier;
    std::vector<Dec*> declarations;

    Def(Specifier * specifier, const std::list<Dec*>& decList);
    ~Def();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct ParamDec: public Node {
    Specifier * specifier;
    VarDec * declarator;

    ParamDec(Specifier * specifier, VarDec * declarator);
    ~ParamDec();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct FunDec: public Node {
    std::string identifier;
    std::vector<ParamDec*> parameters;

    FunDec(const std::string& id, const std::list<ParamDec*>& varList = {});
    ~FunDec();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct Specifier: public Node {};

struct PrimitiveSpecifier final: public Specifier {
    Primitive primitive;

    PrimitiveSpecifier(const std::string& type);
};

struct StructSpecifier final: public Specifier {
    std::string identifier;
    std::vector<Def*> definitions;

    StructSpecifier(const std::string& id, const std::list<Def*>& defList = {});
    ~StructSpecifier();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct ExtDef: public Node {};

struct ExtVarDef final: public ExtDef {
    Specifier * specifier;
    std::vector<VarDec*> varDecs;

    ExtVarDef(Specifier * specifier, const std::list<VarDec*>& extDecList);
    ~ExtVarDef();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct StructDef final: public ExtDef {
    StructSpecifier * specifier;

    StructDef(Specifier * specifier);
    ~StructDef();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct FunDef final: public ExtDef {
    Specifier * specifier;
    FunDec * declarator;
    CompoundStmt * body;

    FunDef(Specifier * specifier, FunDec * declarator, CompoundStmt * body);
    ~FunDef();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct Program final: public Node {
    std::vector<ExtDef*> extDefs;

    Program(const std::list<ExtDef*>& extDefList);
    ~Program();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};


// ------------------------ expressions ------------------------------

struct Exp: public Node {
    std::shared_ptr<Type> type;

    Exp() {}
    Exp(std::shared_ptr<Type> type): type(type) {}
    virtual ~Exp() {}
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
};


struct IdExp final: public Exp {
    std::string identifier;

    IdExp(const std::string&);
};

struct ArrayExp: public Exp {
    Exp * subject, * index;

    ArrayExp(Exp * subject, Exp * index);
    ~ArrayExp();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct MemberExp: public Exp {
    Exp * subject;
    std::string member;

    MemberExp(Exp * subject, const std::string& member);
    ~MemberExp();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
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
    Exp * argument;

    UnaryExp(Operator opt, Exp * argument);
    ~UnaryExp();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct BinaryExp: public Exp {
    Operator opt;
    Exp * left, * right;

    BinaryExp(Exp * left, Operator opt, Exp * right);
    ~BinaryExp();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct AssignExp: public Exp {
    Exp * left, * right;

    AssignExp(Exp * left, Exp * right);
    ~AssignExp();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct CallExp: public Exp {
    std::string identifier;
    std::vector<Exp*> arguments;

    CallExp(const std::string& id, const std::list<Exp*>& arguments = {});
    ~CallExp();
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};


// ------------------------------ statements -------------------------------------

struct Stmt: public Node {};

struct ExpStmt final: public Stmt {
    Exp * expression;

    ExpStmt(Exp * expression);
    ~ExpStmt() {
        delete expression;
    }
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct ReturnStmt final: public Stmt {
    Exp * argument;

    ReturnStmt(Exp * argument = nullptr): argument(argument) {}
    ~ReturnStmt() {
        delete argument;
    }
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct IfStmt final: public Stmt {
    Exp * test;
    Stmt * consequent, * alternate;

    IfStmt(Exp * test, Stmt * consequent, Stmt * alternate = nullptr);
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct WhileStmt final: public Stmt {
    Exp * test;
    Stmt * body;

    WhileStmt(Exp * test, Stmt * body);
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

struct ForStmt final: public Stmt {
    Exp * init, * test, * update;
    Stmt * body;

    ForStmt(Exp * init, Exp * test, Exp * update, Stmt * body);
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
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
    void setScope(std::shared_ptr<SymbolTable>) override;
    void traverse(const std::vector<Walker*>& walkers, Node * parent) override;
};

} // namespace AST

#endif // AST_HPP

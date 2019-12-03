#ifndef AST_H
#define AST_H

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "symbol_table.hpp"
#include "type.hpp"


namespace CST {
    struct Node;
    struct StrNode;
    struct NtNode;
}


/* Expressions */

namespace AST {

struct Node {
    // TODO: line number
    std::shared_ptr<SymbolTable> scope;

    virtual void setScope(std::shared_ptr<SymbolTable> scope) {
        this->scope = scope;
    }
};

struct Exp: public Node {
    std::shared_ptr<Type> type;

    Exp(): type(nullptr) {}
    Exp(std::shared_ptr<Type> type): type(type) {}
    virtual ~Exp() {}
    static std::unique_ptr<Exp> createExp(const CST::Node&);
};

struct LiteralExp final: public Exp {
    union {
        char charVal;
        int intVal;
        float floatVal;
    };

    LiteralExp(const CST::Node&);
};

struct IdExp final: public Exp {
    std::string identifier;

    IdExp(const CST::StrNode&);
};

struct ArrayExp: public Exp {
    std::unique_ptr<Exp> subject, index;

    ArrayExp(const CST::NtNode&);
};

struct MemberExp: public Exp {
    std::unique_ptr<Exp> subject;
    std::unique_ptr<IdExp> member;

    MemberExp(const CST::NtNode&);
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
    enum Operator opt;
    std::unique_ptr<Exp> argument;

    UnaryExp(const CST::NtNode&);
};

struct BinaryExp: public Exp {
    enum Operator opt;
    std::unique_ptr<Exp> left, right;

    BinaryExp(const CST::NtNode&);
};

struct AssignExp: public Exp {
    std::unique_ptr<Exp> left, right;

    AssignExp(const CST::NtNode&);
};

struct CallExp: public Exp {
    std::unique_ptr<IdExp> callee;
    std::vector<std::unique_ptr<Exp>> arguments;

    CallExp(const CST::NtNode&);
};


/* Statements */

struct Stmt: public Node {
    virtual ~Stmt() {}

    // TODO: scope
    static std::unique_ptr<Stmt> createStmt(const CST::NtNode&);
};

struct ExpStmt: public Stmt {
    std::unique_ptr<Exp> expression;

    ExpStmt(const CST::NtNode&);
};

struct ReturnStmt: public Stmt {
    std::unique_ptr<Exp> argument;

    ReturnStmt(const CST::NtNode&);
};

struct IfStmt: public Stmt {
    std::unique_ptr<Exp> test;
    std::unique_ptr<Stmt> consequent, alternate;

    IfStmt(const CST::NtNode&);
    void setScope(std::shared_ptr<SymbolTable>);
};

struct WhileStmt: public Stmt {
    std::unique_ptr<Exp> test;
    std::unique_ptr<Stmt> body;

    WhileStmt(const CST::NtNode&);
    void setScope(std::shared_ptr<SymbolTable>);
};

struct ForStmt: public Stmt {
    std::unique_ptr<Exp> init, test, update;
    std::unique_ptr<Stmt> body;

    ForStmt(const CST::NtNode&);
    void setScope(std::shared_ptr<SymbolTable>);
};

struct VarDef;

struct ComplexStmt: public Stmt {
    std::vector<std::unique_ptr<VarDef>> definitions;
    std::vector<std::unique_ptr<Stmt>> body;

    ComplexStmt(const CST::NtNode&);
    void setScope(std::shared_ptr<SymbolTable>);
};


/* Definitions */

struct Def: public Node {
    std::string identifier;
    std::shared_ptr<Type> type;
};

struct VarDef: public Def {
    std::unique_ptr<Exp> init;

    // VarDef(std::shared_ptr<Type>, const CST::NtNode&);
};

struct StructDef: public Def {
    StructDef(const CST::NtNode&);
};

struct FunctionDef: public Def {
    std::vector<std::unique_ptr<VarDef>> parameters;
    std::unique_ptr<ComplexStmt> body;

    FunctionDef(const CST::NtNode&);
    void setScope(std::shared_ptr<SymbolTable> scope);
};


/* Program */

struct Program: public Node {
    std::vector<std::unique_ptr<Def>> definitions;
    std::shared_ptr<SymbolTable> scope;

    Program(const CST::Node&);

    void initScope() {
        scope = std::shared_ptr<SymbolTable>(new SymbolTable(nullptr));
        for (auto& def: definitions) {
            def->setScope(scope);
        }
    }
};

} // end of namespace AST

#endif

#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include <tuple>
#include <vector>
#include <unordered_set>
#include "ast.hpp"
#include "ast_visitor.hpp"

namespace smt {


class ScopeSetter: public AST::Visitor {
public:
    void defaultEnter(AST::Node *, AST::Node *) override;
    void enter(AST::Program *, AST::Node *) override;
    void enter(AST::FunDef *, AST::Node *) override;
    void enter(AST::ForStmt *, AST::Node *) override;
    void enter(AST::CompoundStmt *, AST::Node *) override;
};


enum SemanticErr {
    ERR_TYPE0,
    ERR_TYPE1,
    ERR_TYPE2,
    ERR_TYPE3,
    ERR_TYPE4,
    ERR_TYPE5,
    ERR_TYPE6,
    ERR_TYPE7,
    ERR_TYPE8,
    ERR_TYPE9,
    ERR_TYPE10,
    ERR_TYPE11,
    ERR_TYPE12,
    ERR_TYPE13,
    ERR_TYPE14,
    ERR_TYPE15
};


struct SemanticErrRecord {
    SemanticErr err;
    AST::Node *cause;
    std::string msg;

    SemanticErrRecord(SemanticErr err, AST::Node *cause, std::string msg):
        err(err), cause(cause), msg(msg) {}
};


class SemanticAnalyzer: public AST::Visitor {
public:
    SemanticAnalyzer(std::vector<SemanticErrRecord>& errStore): errs(errStore) {}

protected:
    void report(SemanticErr errType, AST::Node *cause, const std::string& msg);
    void report(AST::Node *cause);
    bool hasErr(AST::Node * node) const;

private:
    std::vector<SemanticErrRecord>& errs;
    std::unordered_set<unsigned> nodesWithErr;
};


class StructInitializer final: public SemanticAnalyzer {
private:
    std::unordered_map<std::string, Shared<Type>> structures;
public:
    StructInitializer(std::vector<SemanticErrRecord>& errStore):
        SemanticAnalyzer(errStore) {}
    void enter(AST::StructDef *, AST::Node *) override;
    void leave(AST::StructDef *, AST::Node *) override;
    void enter(AST::StructSpecifier *, AST::Node *) override;
};


// after struct initializer finishes its walk
class SymbolSetter final: public SemanticAnalyzer {
private:
    std::unordered_map<unsigned, Shared<Type>> typeRefs;
public:
    SymbolSetter(std::vector<SemanticErrRecord>& errStore):
        SemanticAnalyzer(errStore) {}
    void enter(AST::Program *, AST::Node *) override;
    void enter(AST::ExtVarDef *, AST::Node *) override;
    void enter(AST::ParamDec *, AST::Node *) override;
    void enter(AST::FunDef *, AST::Node *) override;
    void enter(AST::FunDec *, AST::Node *) override;
    void enter(AST::Def *, AST::Node *) override;
    void enter(AST::Dec *, AST::Node *) override;
    void leave(AST::VarDec *, AST::Node *) override;
    void leave(AST::ArrDec *, AST::Node *) override;
};


// currently it synthesizes and checks types
class TypeSynthesizer final: public SemanticAnalyzer {
private:
    std::unordered_map<unsigned, Shared<Type>> funcReturnTypes;
public:
    TypeSynthesizer(std::vector<SemanticErrRecord>& errStore):
        SemanticAnalyzer(errStore) {}
    void leave(AST::IdExp *, AST::Node *) override;
    void leave(AST::CallExp *, AST::Node *) override;
    void leave(AST::AssignExp *, AST::Node *) override;
    void leave(AST::Dec *, AST::Node *) override;
    void leave(AST::UnaryExp *, AST::Node *) override;
    void leave(AST::BinaryExp *, AST::Node *) override;
    void leave(AST::MemberExp *, AST::Node *) override;
    void leave(AST::ArrayExp *, AST::Node *) override;
    void enter(AST::FunDef *, AST::Node *) override;
    void enter(AST::CompoundStmt *, AST::Node *) override;
    void enter(AST::IfStmt *, AST::Node *) override;
    void enter(AST::WhileStmt *, AST::Node *) override;
    void enter(AST::ForStmt *, AST::Node *) override;
    void leave(AST::ReturnStmt *, AST::Node *) override;
};


} // namespace smt

#endif // SEMANTIC_HPP

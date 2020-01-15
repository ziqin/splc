#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include <tuple>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "ast.hpp"
#include "ast_visitor.hpp"
#include "semantic_err.hpp"

namespace smt {

std::vector<SemanticErrRecord> analyzeSemantic(ast::Program *ast);

class ScopeSetter: public ast::Visitor {
public:
    void defaultEnter(ast::Node *, ast::Node *) override;
    void enter(ast::Program *, ast::Node *) override;
    void enter(ast::FunDef *, ast::Node *) override;
    void enter(ast::ForStmt *, ast::Node *) override;
    void enter(ast::CompoundStmt *, ast::Node *) override;
};


class SemanticAnalyzer: public ast::Visitor {
public:
    explicit SemanticAnalyzer(std::vector<SemanticErrRecord>& errStore): errs(errStore) {}

protected:
    void report(SemanticErr errType, ast::Node *cause, const std::string& msg);
    void report(ast::Node *cause);
    bool hasErr(ast::Node * node) const;

private:
    std::vector<SemanticErrRecord>& errs;
    std::unordered_set<unsigned> nodesWithErr;
};


class StructInitializer final: public SemanticAnalyzer {
private:
    std::unordered_map<std::string, Shared<Type>> structures;
public:
    explicit StructInitializer(std::vector<SemanticErrRecord>& errStore):
        SemanticAnalyzer(errStore) {}
    void enter(ast::StructDef *, ast::Node *) override;
    void leave(ast::StructDef *, ast::Node *) override;
    void enter(ast::StructSpecifier *, ast::Node *) override;
};


// after struct initializer finishes its walk
class SymbolSetter final: public SemanticAnalyzer {
private:
    std::unordered_map<unsigned, Shared<Type>> typeRefs;
public:
    explicit SymbolSetter(std::vector<SemanticErrRecord>& errStore):
        SemanticAnalyzer(errStore) {}
    void enter(ast::Program *, ast::Node *) override;
    void enter(ast::ExtVarDef *, ast::Node *) override;
    void enter(ast::ParamDec *, ast::Node *) override;
    void enter(ast::FunDef *, ast::Node *) override;
    void enter(ast::FunDec *, ast::Node *) override;
    void enter(ast::Def *, ast::Node *) override;
    void enter(ast::Dec *, ast::Node *) override;
    void leave(ast::VarDec *, ast::Node *) override;
    void leave(ast::ArrDec *, ast::Node *) override;
};


// currently it synthesizes and checks types
class TypeSynthesizer final: public SemanticAnalyzer {
private:
    std::unordered_map<unsigned, Shared<Type>> funcReturnTypes;
public:
    explicit TypeSynthesizer(std::vector<SemanticErrRecord>& errStore):
        SemanticAnalyzer(errStore) {}
    void leave(ast::IdExp *, ast::Node *) override;
    void leave(ast::CallExp *, ast::Node *) override;
    void leave(ast::AssignExp *, ast::Node *) override;
    void leave(ast::Dec *, ast::Node *) override;
    void leave(ast::UnaryExp *, ast::Node *) override;
    void leave(ast::BinaryExp *, ast::Node *) override;
    void leave(ast::MemberExp *, ast::Node *) override;
    void leave(ast::ArrayExp *, ast::Node *) override;
    void enter(ast::FunDef *, ast::Node *) override;
    void enter(ast::CompoundStmt *, ast::Node *) override;
    void enter(ast::IfStmt *, ast::Node *) override;
    void enter(ast::WhileStmt *, ast::Node *) override;
    void enter(ast::ForStmt *, ast::Node *) override;
    void leave(ast::ReturnStmt *, ast::Node *) override;
};


} // namespace smt

#endif // SEMANTIC_HPP

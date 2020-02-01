#ifndef GEN_TAC_HPP
#define GEN_TAC_HPP

#include <list>
#include <memory>
#include <ostream>
#include <stack>
#include "tac.hpp"
#include "ast.hpp"


namespace ir {

class TacGenerator final: public ast::Visitor {
private:
    std::list<Tac*> codes;
    std::stack<std::shared_ptr<TacOperand>> places;

    std::shared_ptr<TacOperand> retrievePlace() {
        auto place = places.top();
        places.pop();
        return place;
    }

    TacGenerator& operator<<(Tac* tac) {
        codes.push_back(tac);
        return *this;
    }

    TacGenerator& operator<<(std::list<Tac*>&& tacs) {
        codes.splice(codes.end(), tacs);
        return *this;
    }

    void translate(ast::Exp *node, std::shared_ptr<TacOperand> place) {
        places.emplace(std::move(place));
        node->visit(this);
    }

    void translateCondExp(const ast::Exp *exp, ir::LabelTac *labelTrue, ir::LabelTac *labelFalse);
    void translateCondExp(const ast::Exp *exp, std::shared_ptr<TacOperand> place);

public:
    explicit TacGenerator(ast::Program *ast);
    ~TacGenerator() override;

    const std::list<Tac*>& getTac() const;

    void visit(ast::FunDef *) override;
    void visit(ast::Def *) override;
    void visit(ast::LiteralExp *) override;
    void visit(ast::IdExp *) override;
    void visit(ast::UnaryExp *) override;
    void visit(ast::BinaryExp *) override;
    void visit(ast::AssignExp *) override;
    void visit(ast::ArrayExp *) override;
    void visit(ast::MemberExp *) override;
    void visit(ast::CallExp *) override;
    void visit(ast::ExpStmt *) override;
    void visit(ast::ReturnStmt *) override;
    void visit(ast::IfStmt *) override;
    void visit(ast::WhileStmt *) override;
    void visit(ast::ForStmt *) override;
    void visit(ast::CompoundStmt *) override;
};

} // namespace ir

#endif // GEN_TAC_HPP

#ifndef GEN_TAC_HPP
#define GEN_TAC_HPP

#include <list>
#include <ostream>
#include "tac.hpp"
#include "ast_visitor.hpp"


namespace ir {

class TacGenerator final: public ast::Visitor {
private:
    std::list<Tac*> codes;
public:
    ~TacGenerator();
    const std::list<Tac*>& getTac() const;
    void printTac(std::ostream& out) const;

    void leave(ast::FunDef *, ast::Node *) override;
};

} // namespace ir

#endif // GEN_TAC_HPP

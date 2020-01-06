#ifndef TRANSLATE_HPP
#define TRANSLATE_HPP

#include <list>
#include <ostream>
#include "tac.hpp"
#include "ast_visitor.hpp"


namespace gen {

class TacGenerator final: public AST::Visitor {
private:
    std::list<Tac*> codes;
public:
    ~TacGenerator();
    const std::list<Tac*>& getTac() const;
    void printTac(std::ostream& out) const;

    void leave(AST::FunDef *, AST::Node *) override;
};

} // namespace gen

#endif // TRANSLATE_HPP

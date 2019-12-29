#ifndef TRANSLATE_HPP
#define TRANSLATE_HPP

#include <functional>
#include <list>
#include <ostream>
#include <typeindex>
#include <typeinfo>
#include "tac.hpp"

#define ENABLE_HOOK_MACRO
#include "ast_walker.hpp"


namespace gen {

class TacGenerator final: public AST::Walker {
private:
    std::list<Tac*> codes;
public:
    TacGenerator();
    ~TacGenerator();
    const std::list<Tac*>& getTac() const;
    void printTac(std::ostream& out) const;
};

}


#endif // TRANSLATE_HPP

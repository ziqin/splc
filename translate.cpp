#include <typeinfo>
#include "ast.hpp"
#include "translate.hpp"

using namespace AST;
using namespace gen;
using namespace std;


TacGenerator::TacGenerator() {
    BEG_LEAVE_HOOK(FunDef);
        this->codes.splice(this->codes.end(), self->translate());
    END_LEAVE_HOOK(FunDef);
}


TacGenerator::~TacGenerator() {
    for (auto tac: codes) delete tac;
}


const std::list<Tac*>& TacGenerator::getTac() const {
    return codes;
}


void TacGenerator::printTac(ostream& out) const {
    for (auto tac: codes) {
        out << tac->toString() << endl;
    }
}

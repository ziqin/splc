#include "semantic.hpp"

using namespace AST;
using namespace std;


ScopeSetter::ScopeSetter() {
    BEG_ENTER_HOOK(Program);
        self->scope = make_shared<SymbolTable>(nullptr);
    END_ENTER_HOOK(Program);

    BEG_ENTER_HOOK(FunDef);
        self->scope = make_shared<SymbolTable>(parent->scope);
        self->specifier->scope = parent->scope;
    END_ENTER_HOOK(FunDef);

    BEG_ENTER_HOOK(ForStmt);
        self->scope = make_shared<SymbolTable>(parent->scope);
    END_ENTER_HOOK(ForStmt);

    BEG_ENTER_HOOK(CompoundStmt);
        self->scope = make_shared<SymbolTable>(parent->scope);
    END_ENTER_HOOK(CompoundStmt);
}

std::optional<Hook> ScopeSetter::getPreHook(std::type_index type) {
    auto dedicatedHook = Walker::getPreHook(type);
    return dedicatedHook ? dedicatedHook : [](Node * self, Node * parent) {
        if (self->scope == nullptr) { // scope has not been specified by its parent
            self->scope = parent->scope;
        }
    };
}

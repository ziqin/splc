#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include "ast.hpp"

#define ENABLE_HOOK_MACRO
#include "ast_walker.hpp"

namespace AST {

class ScopeSetter: public Walker {
public:
    ScopeSetter();
    std::optional<Hook> getPreHook(std::type_index) override;
};


}

#endif // SEMANTIC_HPP

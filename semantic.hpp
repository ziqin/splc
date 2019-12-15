#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include "ast.hpp"
#include "ast_walker.hpp"

namespace AST {

class DumpWalker: public Walker {
private:
    std::unordered_map<void*, int> indent;

public:
    std::optional<Hook> getPreHook(std::type_index type) override;
};

}

#endif // SEMANTIC_HPP

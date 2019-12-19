#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include <tuple>
#include <vector>
#include <unordered_set>
#include "ast.hpp"

#define ENABLE_HOOK_MACRO
#include "ast_walker.hpp"

namespace AST {


class ScopeSetter: public Walker {
public:
    ScopeSetter();
    std::optional<Hook> getPreHook(std::type_index) override;
};


class SemanticAnalyzer: public Walker {
public:
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
    using SemanticErrRecord = std::tuple<SemanticErr, Node *, std::string>;

    SemanticAnalyzer(std::vector<SemanticErrRecord>& errStore): errs(errStore) {}
    void report(SemanticErr errType, Node * cause, const std::string& msg);
    bool hasErr(Node * node) const;

private:
    std::vector<SemanticErrRecord>& errs;
    std::unordered_set<unsigned> nodesWithErr;
};


class StructInitializer: public SemanticAnalyzer {
private:
    std::unordered_map<std::string, Shared<Type>> structures;
public:
    StructInitializer(std::vector<SemanticErrRecord>&);
};


class SymbolSetter final: public SemanticAnalyzer {
private:
    std::unordered_map<unsigned, Shared<Type>> typeRefs;
public:
    SymbolSetter(std::vector<SemanticErrRecord>&);
};


class TypeSynthesizer final: public SemanticAnalyzer {
public:
    TypeSynthesizer(std::vector<SemanticErrRecord>&);
};


}

#endif // SEMANTIC_HPP

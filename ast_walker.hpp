#ifndef AST_WALKER_HPP
#define AST_WALKER_HPP

#include <functional>
#include <optional>
#include <typeindex>

namespace AST {

struct Node;

using Hook = std::function<void(Node*,Node*)>;

class Walker {
public:
    virtual ~Walker() {}
    virtual std::optional<Hook> getPreHook(std::type_index type) {
        auto hookItr = preHooks.find(type);
        if (hookItr != preHooks.end()) return hookItr->second;
        return std::nullopt;
    }
    virtual std::optional<Hook> getPostHook(std::type_index type) {
        auto hookItr = postHooks.find(type);
        if (hookItr != postHooks.end()) return hookItr->second;
        return std::nullopt;
    }
protected:
    std::unordered_map<std::type_index, Hook> preHooks;
    std::unordered_map<std::type_index, Hook> postHooks;
};

}

#endif // AST_WALKER_HPP

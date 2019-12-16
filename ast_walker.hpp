#ifndef AST_WALKER_HPP
#define AST_WALKER_HPP

#include <functional>
#include <optional>
#include <typeindex>
#include <typeinfo>

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

#ifdef ENABLE_HOOK_MACRO

// `this` refers to the instance of the Walker class
// `self` refers to the AST node being visited
#define BEG_ENTER_HOOK(NodeType) preHooks[typeid(NodeType)] = [this](Node * _current, Node * parent) { \
        auto self = dynamic_cast<NodeType*>(_current)
#define END_ENTER_HOOK(NodeType) }

#define BEG_LEAVE_HOOK(NodeType) postHooks[typeid(NodeType)] = [this](Node * _current, Node * parent) { \
        auto self = dynamic_cast<NodeType*>(_current)
#define END_LEAVE_HOOK(NodeType) }

#endif // ENABLE_HOOK_MACRO


#endif // AST_WALKER_HPP

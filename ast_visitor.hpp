#ifndef AST_VISITOR_HPP
#define AST_VISITOR_HPP

#include "ast.hpp"

namespace AST {

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void defaultVisit(Node *self) {}
    virtual void defaultEnter(Node *self, Node *parent) {}
    virtual void defaultLeave(Node *self, Node *parent) {}

    #define DEFINE_VISITOR_VISIT(T)             \
    virtual void visit(T *self) {               \
        defaultVisit(self);                     \
    }

    #define DEFINE_VISITOR_ENTER(T)             \
    virtual void enter(T *self, Node *parent) { \
        defaultEnter(self, parent);             \
    }

    #define DEFINE_VISITOR_LEAVE(T)             \
    virtual void leave(T *self, Node *parent) { \
        defaultLeave(self, parent);             \
    }

    FOR_EACH_NODE(DEFINE_VISITOR_VISIT)
    FOR_EACH_NODE(DEFINE_VISITOR_ENTER)
    FOR_EACH_NODE(DEFINE_VISITOR_LEAVE)
};

} // namespace AST

#endif // AST_VISITOR_HPP

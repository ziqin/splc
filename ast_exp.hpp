#ifndef AST_EXP_HPP
#define AST_EXP_HPP

#include <initializer_list>
#include <memory>
#include <vector>
#include "type.hpp"


namespace AST {

struct Node {
};


struct Exp: public Node {
    std::shared_ptr<Type> type;

    Exp() {}
    Exp(std::shared_ptr<Type> type): type(type) {}
    virtual ~Exp() {}
};


struct LiteralExp final: public Exp {
    union {
        char charVal;
        int intVal;
        double floatVal;
    };

    LiteralExp(char);
    LiteralExp(int);
    LiteralExp(double);
};


struct IdExp final: public Exp {
    std::string identifier;

    IdExp(const std::string&);
};

struct ArrayExp: public Exp {
    Exp * subject, * index;

    ArrayExp(Exp * subject, Exp * index): subject(subject), index(index) {}
    ~ArrayExp() {
        delete subject;
        delete index;
    }
};

struct MemberExp: public Exp {
    Exp * subject;
    IdExp * member;

    MemberExp(Exp * subject, Exp * member);
    ~MemberExp() {
        delete subject;
        delete member;
    }
};

enum Operator {
    OPT_AND,
    OPT_OR,
    OPT_LT,
    OPT_LE,
    OPT_GT,
    OPT_GE,
    OPT_NE,
    OPT_EQ,
    OPT_PLUS,
    OPT_MINUS,
    OPT_MUL,
    OPT_DIV,
    OPT_NOT
};

struct UnaryExp: public Exp {
    Operator opt;
    Exp * argument;

    UnaryExp(Operator opt, Exp * argument);
    ~UnaryExp() {
        delete argument;
    }
};

struct BinaryExp: public Exp {
    Operator opt;
    Exp * left, * right;

    BinaryExp(Exp * left, Operator opt, Exp * right);
    ~BinaryExp() {
        delete left;
        delete right;
    }
};

struct AssignExp: public Exp {
    Exp * left, * right;

    AssignExp(Exp * left, Exp * right): left(left), right(right) {}
    ~AssignExp() {
        delete left;
        delete right;
    }
};

struct CallExp: public Exp {
    IdExp * callee;
    std::vector<Exp*> arguments;

    CallExp(Exp * callee, std::initializer_list<Exp*> arguments);
    ~CallExp() {
        delete callee;
        for (Exp * arg: arguments) delete arg;
    }
};


} // namespace AST

#endif // AST_EXP_HPP
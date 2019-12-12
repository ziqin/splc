#include <exception>
#include <cctype>
#include "ast_exp.hpp"

using namespace AST;
using namespace std;


LiteralExp::LiteralExp(char val):
    Exp(shared_ptr<Type>(new PrimitiveType(PrimitiveType::TYPE_CHAR))),
    charVal(val) {}

LiteralExp::LiteralExp(int val):
    Exp(shared_ptr<Type>(new PrimitiveType(PrimitiveType::TYPE_INT))),
    intVal(val) {}

LiteralExp::LiteralExp(double val):
    Exp(shared_ptr<Type>(new PrimitiveType(PrimitiveType::TYPE_FLOAT))),
    floatVal(val) {}

IdExp::IdExp(const std::string& id): identifier(id) {
    auto itr = id.begin();
    if (itr == id.end()) throw invalid_argument("id cannot be empty");
    if (!isalpha(*itr) && *itr != '_') throw invalid_argument("illegal id");
    for (++itr; itr != id.end(); ++itr) {
        if (!isalnum(*itr) && *itr != '_') throw invalid_argument("illegal id");
    }
}

MemberExp::MemberExp(Exp * subject, Exp * member):
    subject(subject), member(dynamic_cast<IdExp*>(member))
{
    if (subject == nullptr) {
        delete member;
        throw invalid_argument("subject cannot be null");
    }
    if (this->member == nullptr) {
        delete member;
        throw invalid_argument("type of `member` should be IdExp");
    }
}

UnaryExp::UnaryExp(Operator opt, Exp * argument):
    opt(opt), argument(argument)
{
    if (opt != OPT_MINUS && opt != OPT_NOT && opt != OPT_PLUS) {
        delete argument;
        this->argument = nullptr;
        throw invalid_argument("illegal operator for unary expression");
    }
}

BinaryExp::BinaryExp(Exp * left, Operator opt, Exp * right):
    opt(opt), left(left), right(right)
{
    if (opt == OPT_NOT) {
        delete left;
        delete right;
        this->left = this->right = nullptr;
        throw invalid_argument("illegal operator for binary expression");
    }
}

CallExp::CallExp(Exp * callee, initializer_list<Exp*> arguments):
    callee(dynamic_cast<IdExp*>(callee)), arguments(arguments)
{
    if (this->callee == nullptr) {
        delete callee;
        for (auto arg: arguments) delete arg;
        throw invalid_argument("type of `callee` should be IdExp");
    }
    for (const auto& argument: this->arguments) {
        if (argument == nullptr) {
            delete callee;
            for (auto arg: arguments) delete arg;
            throw invalid_argument("argument cannot be null");
        }
    }
}

#ifndef AST_DUMP_HPP
#define AST_DUMP_HPP

#include <ostream>
#include <string>
#include <typeinfo>
#include "ast.hpp"

#define ENABLE_HOOK_MACRO
#include "ast_walker.hpp"


#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
template <class T>
std::string type(const T& t) {
    const char * name = typeid(t).name();
    int status = 0;
    char * _demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    if (status == 0) {
        std::string demangled(_demangled);
        std::free(_demangled);
        return demangled;
    }
    return name;
}
#else
template <class T>
std::string type(const T& t) {
    return typeid(t).name();
}
#endif


namespace AST {

std::ostream& operator<<(std::ostream& out, const Location& loc) {
    return out << "@[" << loc.start.line << '.' << loc.start.column
        << '~' << loc.end.line << '.' << loc.end.column << ']';
}


class DumpWalker: public Walker {
private:
    std::ostream& outStream;
    std::unordered_map<unsigned, unsigned> indents;

    std::ostream& out(Node * self, Node * parent) {
        unsigned indent = parent ? indents[parent->id] + 2 : 0;
        indents[self->id] = indent;
        for (unsigned i = 0; i < indent; ++i) outStream << ' ';
        outStream << type(*self) << ' ' << self->loc;
        return outStream;
    }

public:
    DumpWalker(std::ostream& outStream): outStream(outStream) {

        BEG_ENTER_HOOK(Program);
            this->out(self, parent) << std::endl;
        END_ENTER_HOOK(Program);

        BEG_ENTER_HOOK(VarDec);
            this->out(self, parent) << " name: " << self->identifier << std::endl;
        END_ENTER_HOOK(VarDec);

        BEG_ENTER_HOOK(ArrDec);
            auto& out = this->out(self, parent) << " dim: ";
            for (int i: self->dimensions) out << '[' << i <<']';
            out << std::endl;
        END_ENTER_HOOK(ArrDec);

        BEG_ENTER_HOOK(FunDec);
            this->out(self, parent) << " name: " << self->identifier << std::endl;
        END_ENTER_HOOK(FunDec);

        BEG_ENTER_HOOK(PrimitiveSpecifier);
            auto& out = this->out(self, parent) << " type: ";
            switch (self->primitive) {
                case TYPE_CHAR: out << "char"; break;
                case TYPE_INT: out << "int"; break;
                case TYPE_FLOAT: out << "float"; break;
                default: out << "auto"; break;
            }
            out << std::endl;
        END_ENTER_HOOK(PrimitiveSpecifier);

        BEG_ENTER_HOOK(StructSpecifier);
            this->out(self, parent) << " name: " << self->identifier << std::endl;
        END_ENTER_HOOK(StructSpecifier);

        BEG_ENTER_HOOK(IdExp);
            this->out(self, parent) << " name: " << self->identifier << std::endl;
        END_ENTER_HOOK(IdExp);

        BEG_ENTER_HOOK(MemberExp);
            this->out(self, parent) << " member: " << self->member << std::endl;
        END_ENTER_HOOK(MemberExp);

        BEG_ENTER_HOOK(UnaryExp);
            auto& out = this->out(self, parent) << " operator: ";
            switch (self->opt) {
                case OPT_PLUS:  out << '+';     break;
                case OPT_MINUS: out << '-';     break;
                case OPT_NOT:   out << '!';     break;
                default:        out << "error"; break;
            }
            out << std::endl;
        END_ENTER_HOOK(UnaryExp);

        BEG_ENTER_HOOK(BinaryExp);
            auto& out = this->out(self, parent) << " operator: ";
            switch (self->opt) {
                case OPT_AND:    out << "&&";    break;
                case OPT_OR:     out << "||";    break;
                case OPT_LT:     out << "<";     break;
                case OPT_LE:     out << "<=";    break;
                case OPT_GT:     out << ">";     break;
                case OPT_GE:     out << ">=";    break;
                case OPT_NE:     out << "!=";    break;
                case OPT_EQ:     out << "==";    break;
                case OPT_PLUS:   out << "+";     break;
                case OPT_MINUS:  out << "-";     break;
                case OPT_MUL:    out << "*";     break;
                case OPT_DIV:    out << "/";     break;
                default:    out << "error"; break;
            }
            out << std::endl;
        END_ENTER_HOOK(BinaryExp);

        BEG_ENTER_HOOK(CallExp);
            this->out(self, parent) << " name: " << self->identifier << std::endl;
        END_ENTER_HOOK(CallExp);
    }

    std::optional<Hook> getPreHook(std::type_index type) {
        auto dedicatedHook = Walker::getPreHook(type);
        return dedicatedHook ? dedicatedHook : [this](Node * self, Node * parent) {
            this->out(self, parent) << std::endl;
        };
    }
};

}

#endif // AST_DUMP_HPP

#ifndef AST_DUMP_HPP
#define AST_DUMP_HPP

#include <ostream>
#include <string>
#include "ast.hpp"

#define ENABLE_HOOK_MACRO
#include "ast_walker.hpp"
#include "utils.hpp"


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
        outStream << fullTypeName(*self) << ' ' << self->loc;
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
                case smt::TYPE_CHAR: out << "char"; break;
                case smt::TYPE_INT: out << "int"; break;
                case smt::TYPE_FLOAT: out << "float"; break;
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

        BEG_ENTER_HOOK(LiteralExp);
            auto& out = this->out(self, parent) << " value: ";
            smt::Primitive primitive = dynamic_cast<const smt::PrimitiveType&>(self->type.value()).primitive;
            switch (primitive) {
                case smt::TYPE_CHAR: out << '\'' << self->charVal << '\''; break;
                case smt::TYPE_INT: out << self->intVal; break;
                case smt::TYPE_FLOAT: out << self->floatVal; break;
                default: break;
            }
            out << std::endl;
        END_ENTER_HOOK(LiteralExp);

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

    std::optional<Hook> getEnterHook(std::type_index type) override {
        auto dedicatedHook = Walker::getEnterHook(type);
        return dedicatedHook ? dedicatedHook : [this](Node * self, Node * parent) {
            this->out(self, parent) << std::endl;
        };
    }
};

}

#endif // AST_DUMP_HPP

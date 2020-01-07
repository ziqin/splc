#ifndef AST_DUMP_HPP
#define AST_DUMP_HPP

#include <ostream>
#include <string>
#include "ast.hpp"
#include "ast_visitor.hpp"
#include "utils.hpp"


namespace ast {

std::ostream& operator<<(std::ostream& out, const Location& loc) {
    return out << "@[" << loc.start.line << '.' << loc.start.column
        << '~' << loc.end.line << '.' << loc.end.column << ']';
}


class Printer: public Visitor {
private:
    std::ostream& outStream;
    std::unordered_map<unsigned, unsigned> indents;

    std::ostream& out(Node * self, Node * parent) {
        unsigned indent = parent ? indents[parent->nodeId] + 2 : 0;
        indents[self->nodeId] = indent;
        for (unsigned i = 0; i < indent; ++i) outStream << ' ';
        outStream << fullTypeName(*self) << ' ' << self->loc;
        return outStream;
    }

public:
    Printer(std::ostream& outStream): outStream(outStream) {}

    void enter(Program *self, Node *parent) override {
        out(self, parent) << std::endl;
    }

    void enter(VarDec *self, Node *parent) override {
        out(self, parent) << " name: " << self->identifier << std::endl;
    }

    void enter(ArrDec *self, Node *parent) override {
        auto& out = this->out(self, parent) << " dim: ";
        for (int i: self->dimensions) out << '[' << i <<']';
        out << std::endl;
    }

    void enter(FunDec *self, Node *parent) override {
        out(self, parent) << " name: " << self->identifier << std::endl;
    }

    void enter(PrimitiveSpecifier *self, Node *parent) override {
        auto& out = this->out(self, parent) << " type: ";
        switch (self->primitive) {
            case smt::TYPE_CHAR: out << "char"; break;
            case smt::TYPE_INT: out << "int"; break;
            case smt::TYPE_FLOAT: out << "float"; break;
            default: out << "auto"; break;
        }
        out << std::endl;    
    }

    void enter(StructSpecifier *self, Node *parent) override {
        out(self, parent) << " name: " << self->identifier << std::endl;
    }

    void enter(IdExp *self, Node *parent) override {
        out(self, parent) << " name: " << self->identifier << std::endl;
    }

    void enter(MemberExp *self, Node *parent) override {
        out(self, parent) << " member: " << self->member << std::endl;
    }

    void enter(LiteralExp *self, Node *parent) override {
        auto& out = this->out(self, parent) << " value: ";
        smt::Primitive primitive = dynamic_cast<const smt::PrimitiveType&>(self->type.value()).primitive;
        switch (primitive) {
            case smt::TYPE_CHAR: out << '\'' << self->charVal << '\''; break;
            case smt::TYPE_INT: out << self->intVal; break;
            case smt::TYPE_FLOAT: out << self->floatVal; break;
            default: break;
        }
        out << std::endl;
    }

    void enter(UnaryExp *self, Node *parent) override {
        auto& out = this->out(self, parent) << " operator: ";
        switch (self->opt) {
            case OPT_PLUS:  out << '+';     break;
            case OPT_MINUS: out << '-';     break;
            case OPT_NOT:   out << '!';     break;
            default:        out << "error"; break;
        }
        out << std::endl;
    }

    void enter(BinaryExp *self, Node *parent) override {
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
    }

    void enter(CallExp *self, Node *parent) override {
        out(self, parent) << " name: " << self->identifier << std::endl;
    }

    void defaultEnter(Node *self, Node *parent) override {
        out(self, parent) << std::endl;
    }
};

}

#endif // AST_DUMP_HPP

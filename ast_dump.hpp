#ifndef AST_DUMP_HPP
#define AST_DUMP_HPP

#include <ostream>
#include <string>
#include <unordered_map>
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
            case smt::Primitive::CHAR: out << "char"; break;
            case smt::Primitive::INT: out << "int"; break;
            case smt::Primitive::FLOAT: out << "float"; break;
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
            case smt::Primitive::CHAR: out << '\'' << self->charVal << '\''; break;
            case smt::Primitive::INT: out << self->intVal; break;
            case smt::Primitive::FLOAT: out << self->floatVal; break;
            default: break;
        }
        out << std::endl;
    }

    void enter(UnaryExp *self, Node *parent) override {
        auto& out = this->out(self, parent) << " operator: ";
        switch (self->opt) {
            case Operator::PLUS:  out << '+';     break;
            case Operator::MINUS: out << '-';     break;
            case Operator::NOT:   out << '!';     break;
            default:        out << "error"; break;
        }
        out << std::endl;
    }

    void enter(BinaryExp *self, Node *parent) override {
        auto& out = this->out(self, parent) << " operator: ";
        switch (self->opt) {
            case Operator::AND:    out << "&&";    break;
            case Operator::OR:     out << "||";    break;
            case Operator::LT:     out << "<";     break;
            case Operator::LE:     out << "<=";    break;
            case Operator::GT:     out << ">";     break;
            case Operator::GE:     out << ">=";    break;
            case Operator::NE:     out << "!=";    break;
            case Operator::EQ:     out << "==";    break;
            case Operator::PLUS:   out << "+";     break;
            case Operator::MINUS:  out << "-";     break;
            case Operator::MUL:    out << "*";     break;
            case Operator::DIV:    out << "/";     break;
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

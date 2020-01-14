#include <typeinfo>
#include "ast.hpp"
#include "gen_tac.hpp"

using namespace ast;
using namespace ir;
using namespace std;


TacGenerator::TacGenerator(ast::Program *ast) {
    for (auto def: ast->extDefs) {
        auto funcDef = dynamic_cast<ast::FunDef*>(def);
        if (funcDef == nullptr) continue;
        funcDef->visit(this);
    }
}

TacGenerator::~TacGenerator() {
    for (auto tac: codes) delete tac;
}

const list<Tac*>& TacGenerator::getTac() const {
    return codes;
}

// TODO: handle non-integer arguments
void TacGenerator::visit(FunDef *self) {
    *this << new FuncTac(self->declarator->identifier);
    for (auto param: self->declarator->parameters) {
        auto place = makeTacOp<VariableOperand>(param->scope->getId(param->declarator->identifier));
        *this << new ParamTac(place);
    }
    self->body->visit(this);
}

void TacGenerator::visit(Def *self) {
    for (auto dec: self->declarations) {
        if (dec->init != nullptr) {
            auto variable = makeTacOp<VariableOperand>(self->scope->getId(dec->declarator->identifier));
            auto tp = makeTacOp<VariableOperand>(self->scope->createPlace());
            translate(dec->init, tp);
            *this << new AssignTac(variable, tp);
        }
    }
}

void TacGenerator::visit(LiteralExp *self) {
    shared_ptr<TacOperand> value;
    switch (smt::as<smt::PrimitiveType>(self->type).primitive) {
    case smt::Primitive::INT:
        value = makeTacOp<ConstantOperand<int>>(self->intVal);
        break;
    case smt::Primitive::CHAR:
        value = makeTacOp<ConstantOperand<char>>(self->charVal);
        break;
    case smt::Primitive::FLOAT:
        value = makeTacOp<ConstantOperand<float>>(self->floatVal);
        break;
    default:
        throw invalid_argument("invalid type");
    }
    *this << new AssignTac(retrievePlace(), value);
}

void TacGenerator::visit(IdExp *self) {
    auto variable = makeTacOp<VariableOperand>(self->scope->getId(self->identifier));
    *this << new AssignTac(retrievePlace(), variable);
}

void TacGenerator::visit(UnaryExp *self) {
    auto place = retrievePlace();
    if (self->opt == Operator::NOT) {
        translateCondExp(self, place);
    } else {
        auto tp = makeTacOp<VariableOperand>(self->scope->createPlace());
        translate(self->argument, tp);
        if (self->opt == Operator::MINUS) {
            *this << new SubTac(place, makeTacOp<ConstantOperand<int>>(0), tp);
        } else if (self->opt == Operator::PLUS) {
            *this << new AssignTac(place, tp);
        } else {
            throw runtime_error("invalid unary operator encountered");
        }
    }
}

void TacGenerator::visit(BinaryExp *self) {
    auto place = retrievePlace();
    switch (self->opt) {
    case Operator::AND:
    case Operator::OR:
    case Operator::LT:
    case Operator::LE:
    case Operator::GT:
    case Operator::GE:
    case Operator::NE:
    case Operator::EQ:
        translateCondExp(self, place);
        break;
    default: {
        auto t1 = makeTacOp<VariableOperand>(self->scope->createPlace());
        auto t2 = makeTacOp<VariableOperand>(self->scope->createPlace());
        translate(self->left, t1);
        translate(self->right, t2);
        switch (self->opt) {
        case Operator::PLUS:
            *this << new AddTac(place, t1, t2);
            break;
        case Operator::MINUS:
            *this << new SubTac(place, t1, t2);
            break;
        case Operator::MUL:
            *this << new MulTac(place, t1, t2);
            break;
        case Operator::DIV:
            *this << new DivTac(place, t1, t2);
            break;
        default:
            throw runtime_error("Invalid binary operator");
        }
    }
    }
}

void TacGenerator::visit(AssignExp *self) {
    auto place = retrievePlace();
    auto lvalue = dynamic_cast<const IdExp*>(self->left);
    if (lvalue == nullptr) {
        throw runtime_error("assignment to non-IdExp has not been implemented");
    }
    // TODO: support assignment of non-integer values
    auto variable = makeTacOp<VariableOperand>(self->scope->getId(lvalue->identifier));
    auto tp = makeTacOp<VariableOperand>(self->scope->createPlace());
    translate(self->right, tp);
    *this << new AssignTac(variable, tp) << new AssignTac(place, variable);
}

void TacGenerator::visit(ArrayExp *self) {
    throw runtime_error("not implemented");
}

void TacGenerator::visit(MemberExp *self) {
    throw runtime_error("not implemented");
}


// TODO: enforce argument number for built-in I/O functions during semantic analysis
void TacGenerator::visit(CallExp *self) {
    auto place = retrievePlace();
    if (self->identifier == "read") {
        *this << new ReadTac(place);
    } else if (self->identifier == "write") {
        auto tp = makeTacOp<VariableOperand>(self->scope->createPlace());
        translate(self->arguments[0], tp);
        *this << new WriteTac(tp);
    } else {
        vector<shared_ptr<TacOperand>> argPlaces;
        // left-to-right evaluation
        for (auto arg: self->arguments) {
            auto argPlace = makeTacOp<VariableOperand>(self->scope->createPlace());
            translate(arg, argPlace);
            argPlaces.push_back(argPlace);
        }
        // push args into stack from right to left
        for (auto argPlace = argPlaces.rbegin(); argPlace != argPlaces.rend(); ++argPlace) {
            *this << new ArgTac(*argPlace);
        }
        *this << new CallTac(place, self->identifier);
    }
}

void TacGenerator::visit(ExpStmt *self) {
    auto tp = makeTacOp<VariableOperand>(self->scope->createPlace());
    translate(self->expression, tp);
}

void TacGenerator::visit(ReturnStmt *self) {
    auto tp = makeTacOp<VariableOperand>(self->scope->createPlace());
    translate(self->argument, tp);
    *this << new ReturnTac(tp);
}

void TacGenerator::visit(IfStmt *self) {
    LabelTac *label1, *label2, *label3;
    label1 = new LabelTac(self->scope->createLabel());
    label2 = new LabelTac(self->scope->createLabel());
    translateCondExp(self->test, label1, label2);
    *this << label1;
    self->consequent->visit(this);
    if (self->alternate != nullptr) {
        label3 = new LabelTac(self->scope->createLabel());
        *this << new GotoTac(label3->no);
    }
    *this << label2;
    if (self->alternate != nullptr) {
        self->alternate->visit(this);
        *this << label3;
    }
}

void TacGenerator::visit(WhileStmt *self) {
    auto label1 = new LabelTac(self->scope->createLabel());
    auto label2 = new LabelTac(self->scope->createLabel());
    auto label3 = new LabelTac(self->scope->createLabel());
    *this << label1;
    translateCondExp(self->test, label2, label3);
    *this << label2;
    self->body->visit(this);
    *this << new GotoTac(label1->no) << label3;
}

void TacGenerator::visit(ForStmt *self) {
    throw runtime_error("not implemented");
}

void TacGenerator::visit(CompoundStmt *self) {
    for (auto def: self->definitions) {
        def->visit(this);
    }
    for (auto stmt: self->body) {
        stmt->visit(this);
    }
}

void TacGenerator::translateCondExp(const Exp *exp, LabelTac *labelTrue, LabelTac *labelFalse) {
    if (typeid(*exp) == typeid(UnaryExp)) {
        translateCondExp(exp, labelFalse, labelTrue);
    } else {
        const BinaryExp *binExp = dynamic_cast<const BinaryExp*>(exp);
        if (binExp == nullptr) {
            throw invalid_argument("invalid expression"); // FIXME: memory leak
        }
        switch (binExp->opt) {
        case Operator::AND: {
            auto label1 = new LabelTac(exp->scope->createLabel());
            translateCondExp(binExp->left, label1, labelFalse);
            *this << label1;
            translateCondExp(binExp->right, labelTrue, labelFalse);
        } break;
        case Operator::OR: {
            auto label1 = new LabelTac(exp->scope->createLabel());
            translateCondExp(binExp->left, labelTrue, label1);
            *this << label1;
            translateCondExp(binExp->right, labelTrue, labelFalse);
        } break;
        default: {
            auto t1 = makeTacOp<VariableOperand>(exp->scope->createPlace());
            auto t2 = makeTacOp<VariableOperand>(exp->scope->createPlace());
            translate(binExp->left, t1);
            translate(binExp->right, t2);
            IfGotoTac *ifGotoTac;
            switch (binExp->opt) {
            case Operator::LT:
                ifGotoTac = new IfLtGotoTac(t1, t2, labelTrue->no);
                break;
            case Operator::LE:
                ifGotoTac = new IfLeGotoTac(t1, t2, labelTrue->no);
                break;
            case Operator::GT:
                ifGotoTac = new IfGtGotoTac(t1, t2, labelTrue->no);
                break;
            case Operator::GE:
                ifGotoTac = new IfGeGotoTac(t1, t2, labelTrue->no);
                break;
            case Operator::NE:
                ifGotoTac = new IfNeGotoTac(t1, t2, labelTrue->no);
                break;
            case Operator::EQ:
                ifGotoTac = new IfEqGotoTac(t1, t2, labelTrue->no);
                break;
            default:
                throw runtime_error("invalid binary expression");
            }
            *this << ifGotoTac << new GotoTac(labelFalse->no);
        }
        }
    }
}

void TacGenerator::translateCondExp(const Exp *exp, std::shared_ptr<TacOperand> place) {
    auto label1 = new LabelTac(exp->scope->createLabel());
    auto label2 = new LabelTac(exp->scope->createLabel());
    *this << new AssignTac(place, makeTacOp<ConstantOperand<int>>(0));
    translateCondExp(exp, label1, label2);
    *this << label1 << new AssignTac(place, makeTacOp<ConstantOperand<int>>(1)) << label2;
}

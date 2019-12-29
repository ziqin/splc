#include <cctype>
#include <exception>
#include <typeinfo>
#include "ast.hpp"
#include "ast_walker.hpp"
#include "syntax.tab.h"
#include "type.hpp"

using namespace AST;
using namespace std;
using namespace gen;


template <typename T>
inline static void execPreHook(const vector<Walker*>& walkers, T * self, Node * parent) {
    for (Walker * walker: walkers) {
        auto hook = walker->getEnterHook(typeid(*self));
        if (hook) hook.value()(self, parent);
    }
}

template <typename T>
inline static void execPostHook(const vector<Walker*>& walkers, T * self, Node * parent) {
    for (Walker * walker: walkers) {
        auto hook = walker->getLeaveHook(typeid(*self));
        if (hook) hook.value()(self, parent);
    }
}

Node::Node() {
    static int gid = 0;
    nodeId = gid++;
}

void Node::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    execPostHook(walkers, this, parent);
}

void Node::setLocation(const YYLTYPE * location) {
    loc.start.line = location->first_line;
    loc.start.column = location->first_column;
    loc.end.line = location->last_line;
    loc.end.column = location->last_column;
}


// ----------------------------- misc --------------------------------

ArrDec::ArrDec(const VarDec& declarator, int dim): VarDec(declarator.identifier) {
    try {
        auto& prefix = dynamic_cast<const ArrDec&>(declarator);
        dimensions = prefix.dimensions;
    } catch (const bad_cast& e) {}
    dimensions.push_back(dim);
}


Dec::Dec(VarDec * declarator, Exp * init): declarator(declarator), init(init) {
    if (declarator == nullptr) throw invalid_argument("declarator cannot be null");
}

Dec::~Dec() {
    deleteAll(declarator, init);
}


void Dec::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    declarator->traverse(walkers, this);
    if (init) init->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}


Def::Def(Specifier * specifier, const list<Dec*>& decList):
    specifier(specifier), declarations(decList.begin(), decList.end())
{
    if (hasNull(this->specifier, this->declarations)) {
        deleteAll(this->specifier, this->declarations);
        throw invalid_argument("specifier/declarations cannot be null");
    }
}

Def::~Def() {
    deleteAll(specifier, declarations);
}

void Def::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    specifier->traverse(walkers, this);
    for (auto dec: declarations) dec->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}


ParamDec::ParamDec(Specifier * specifier, VarDec * declarator):
    specifier(specifier), declarator(declarator)
{
    if (hasNull(this->specifier, this->declarator)) {
        deleteAll(this->specifier, this->declarator);
        throw invalid_argument("specifier/declarator cannot be null");
    }
}

ParamDec::~ParamDec() {
    deleteAll(specifier, declarator);
}

void ParamDec::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    specifier->traverse(walkers, this);
    declarator->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}


FunDec::FunDec(const string& identifier, const list<ParamDec*>& varList):
    identifier(identifier), parameters(varList.begin(), varList.end())
{
    if (hasNull(this->parameters)) {
        deleteAll(this->parameters);
        throw invalid_argument("parameters cannot be null");
    }
}

FunDec::~FunDec() {
    deleteAll(parameters);
}

void FunDec::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    for (auto para: parameters) para->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}


PrimitiveSpecifier::PrimitiveSpecifier(const string& typeName) {
    if (typeName == "char") {
        primitive = smt::TYPE_CHAR;
        type.set(new smt::PrimitiveType(smt::TYPE_CHAR));
    } else if (typeName == "int") {
        primitive = smt::TYPE_INT;
        type.set(new smt::PrimitiveType(smt::TYPE_INT));
    } else if (typeName == "float") {
        primitive = smt::TYPE_FLOAT;
        type.set(new smt::PrimitiveType(smt::TYPE_FLOAT));
    } else {
        throw invalid_argument("illegal primitive type");
    }
}


StructSpecifier::StructSpecifier(const string& identifier, const list<Def*>& defList):
    identifier(identifier), definitions(defList.begin(), defList.end())
{
    if (hasNull(this->definitions)) {
        deleteAll(this->definitions);
        throw invalid_argument("definitions cannot be null");
    }
    type.set(new smt::StructType);
}

StructSpecifier::~StructSpecifier() {
    deleteAll(definitions);
}

void StructSpecifier::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    for (auto def: definitions) def->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}


ExtVarDef::ExtVarDef(Specifier * specifier, const list<VarDec*>& extDecList):
    specifier(specifier), varDecs(extDecList.begin(), extDecList.end())
{
    if (hasNull(this->specifier, this->varDecs)) {
        deleteAll(this->specifier, this->varDecs);
        throw invalid_argument("specifier cannot be null");
    }
}

ExtVarDef::~ExtVarDef() {
    deleteAll(specifier, varDecs);
}


void ExtVarDef::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    specifier->traverse(walkers, this);
    for (auto dec: varDecs) dec->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

StructDef::StructDef(Specifier * specifier):
    specifier(dynamic_cast<StructSpecifier*>(specifier))
{
    if (specifier == nullptr) throw invalid_argument("type of `specifier` should be StructSpecifier");
}

StructDef::~StructDef() {
    delete specifier;
}

void StructDef::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    specifier->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}


FunDef::FunDef(Specifier * specifier, FunDec * declarator, CompoundStmt * body):
    specifier(specifier), declarator(declarator), body(body)
{
    if (hasNull(this->specifier, this->declarator, this->body)) {
        deleteAll(this->specifier, this->declarator, this->body);
        throw invalid_argument("specifier/declarator/body cannot be null");
    }
}

FunDef::~FunDef() {
    deleteAll(specifier, declarator, body);
}

void FunDef::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    specifier->traverse(walkers, this);
    declarator->traverse(walkers, this);
    body->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

// TODO: handle non-integer arguments
list<Tac*> FunDef::translate() {
    list<Tac*> code { new FuncTac(declarator->identifier) };
    for (auto param: declarator->parameters) {
        auto place = makeTacOp<VariableOperand>(param->scope->getId(param->declarator->identifier));
        code.push_back(new ParamTac(place));
    }
    code.splice(code.end(), body->translate());
    return code;
}


Program::Program(const list<ExtDef*>& extDefList):
    extDefs(extDefList.begin(), extDefList.end())
{
    if (hasNull(extDefs)) {
        deleteAll(extDefs);
        throw invalid_argument("extDefs cannot be null");
    }
}

Program::~Program() {
    deleteAll(extDefs);
}

void Program::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    for (auto def: extDefs) def->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

// ----------------------- expressions ---------------------------

list<Tac*> translateCondExp(const Exp *exp, LabelTac *labelTrue, LabelTac *labelFalse) {
    if (typeid(*exp) == typeid(UnaryExp)) {
        return translateCondExp(exp, labelFalse, labelTrue);
    } else {
        const BinaryExp *binExp = dynamic_cast<const BinaryExp*>(exp);
        if (binExp == nullptr) {
            throw invalid_argument("invalid expression"); // FIXME: memory leak
        }
        list<Tac*> code;
        switch (binExp->opt) {
        case OPT_AND: {
            auto label1 = new LabelTac(exp->scope->createLabel());
            code.splice(code.end(), translateCondExp(binExp->left, label1, labelFalse));
            code.push_back(label1);
            code.splice(code.end(), translateCondExp(binExp->right, labelTrue, labelFalse));
        } break;
        case OPT_OR: {
            auto label1 = new LabelTac(exp->scope->createLabel());
            code.splice(code.end(), translateCondExp(binExp->left, labelTrue, label1));
            code.push_back(label1);
            code.splice(code.end(), translateCondExp(binExp->right, labelTrue, labelFalse));
        } break;
        default: {
            auto t1 = makeTacOp<VariableOperand>(exp->scope->createPlace());
            auto t2 = makeTacOp<VariableOperand>(exp->scope->createPlace());
            code.splice(code.end(), binExp->left->translate(t1));
            code.splice(code.end(), binExp->right->translate(t2));
            IfGotoTac *ifGotoTac;
            switch (binExp->opt) {
            case OPT_LT:
                ifGotoTac = new IfLtGotoTac(t1, t2, labelTrue->no);
                break;
            case OPT_LE:
                ifGotoTac = new IfLeGotoTac(t1, t2, labelTrue->no);
                break;
            case OPT_GT:
                ifGotoTac = new IfGtGotoTac(t1, t2, labelTrue->no);
                break;
            case OPT_GE:
                ifGotoTac = new IfGeGotoTac(t1, t2, labelTrue->no);
                break;
            case OPT_NE:
                ifGotoTac = new IfNeGotoTac(t1, t2, labelTrue->no);
                break;
            case OPT_EQ:
                ifGotoTac = new IfEqGotoTac(t1, t2, labelTrue->no);
                break;
            default:
                for (auto tac: code) delete tac;
                throw runtime_error("invalid binary expression");
            }
            code.push_back(ifGotoTac);
            code.push_back(new GotoTac(labelFalse->no));
        } break;
        }
        return code;
    }
}

list<Tac*> translateCondExp(const Exp *exp, std::shared_ptr<TacOperand> place) {
    auto label1 = new LabelTac(exp->scope->createLabel());
    auto label2 = new LabelTac(exp->scope->createLabel());
    list<Tac*> code { new AssignTac(place, makeTacOp<ConstantOperand<int>>(0)) };
    code.splice(code.end(), move(translateCondExp(exp, label1, label2)));
    code.push_back(label1);
    code.push_back(new AssignTac(place, makeTacOp<ConstantOperand<int>>(1)));
    code.push_back(label2);
    return code;
}


LiteralExp::LiteralExp(char val):
    Exp(Shared<smt::Type>(new smt::PrimitiveType(smt::TYPE_CHAR))),
    charVal(val) {}

LiteralExp::LiteralExp(int val):
    Exp(Shared<smt::Type>(new smt::PrimitiveType(smt::TYPE_INT))),
    intVal(val) {}

LiteralExp::LiteralExp(double val):
    Exp(Shared<smt::Type>(new smt::PrimitiveType(smt::TYPE_FLOAT))),
    floatVal(val) {}

list<Tac*> LiteralExp::translate(shared_ptr<TacOperand> place) {
    shared_ptr<TacOperand> value;
    switch (smt::as<smt::PrimitiveType>(type).primitive) {
    case smt::TYPE_INT:
        value = makeTacOp<ConstantOperand<int>>(intVal);
        break;
    case smt::TYPE_CHAR:
        value = makeTacOp<ConstantOperand<char>>(charVal);
        break;
    case smt::TYPE_FLOAT:
        value = makeTacOp<ConstantOperand<float>>(floatVal);
        break;
    default:
        throw invalid_argument("invalid type");
    }
    return { new AssignTac(place, value) };
}


IdExp::IdExp(const string& identifier): identifier(identifier) {
    auto itr = identifier.begin();
    if (itr == identifier.end()) throw invalid_argument("identifier cannot be empty");
    if (!isalpha(*itr) && *itr != '_') throw invalid_argument("illegal identifier");
    for (++itr; itr != identifier.end(); ++itr) {
        if (!isalnum(*itr) && *itr != '_') throw invalid_argument("illegal identifier");
    }
}

list<Tac*> IdExp::translate(shared_ptr<TacOperand> place) {
    auto variable = makeTacOp<VariableOperand>(scope->getId(identifier));
    return { new AssignTac(place, variable) };
}

ArrayExp::ArrayExp(Exp * subject, Exp * index): subject(subject), index(index) {
    if (hasNull(this->subject, this->index)) {
        deleteAll(this->subject, this->index);
        throw invalid_argument("subject/index cannot be null");
    }
}

ArrayExp::~ArrayExp() {
    deleteAll(subject, index);
}

void ArrayExp::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    subject->traverse(walkers, this);
    index->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}


MemberExp::MemberExp(Exp * subject, const string& member):
    subject(subject), member(member)
{
    if (this->subject == nullptr) throw invalid_argument("subject cannot be null");
}

MemberExp::~MemberExp() {
    delete subject;
}


void MemberExp::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    subject->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}


UnaryExp::UnaryExp(Operator opt, Exp * argument):
    opt(opt), argument(argument)
{
    if (opt != OPT_MINUS && opt != OPT_NOT && opt != OPT_PLUS) {
        delete argument;
        throw invalid_argument("illegal operator for unary expression");
    }
}

UnaryExp::~UnaryExp() {
    delete argument;
}

void UnaryExp::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    argument->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

list<Tac*> UnaryExp::translate(shared_ptr<TacOperand> place) {
    if (opt == OPT_NOT) {
        return translateCondExp(this, place);
    } else {
        auto tp = makeTacOp<VariableOperand>(scope->createPlace());
        auto code = argument->translate(tp);
        if (opt == OPT_MINUS) {
            code.push_back(new SubTac(place, makeTacOp<ConstantOperand<int>>(0), tp));
        } else if (opt == OPT_PLUS) {
            code.push_back(new AssignTac(place, tp));
        } else {
            throw runtime_error("invalid unary operator encountered");
        }
        return code;
    }
}


BinaryExp::BinaryExp(Exp * left, Operator opt, Exp * right):
    opt(opt), left(left), right(right)
{
    if (opt == OPT_NOT) {
        deleteAll(this->left, this->right);
        throw invalid_argument("illegal operator for binary expression");
    }
}

BinaryExp::~BinaryExp() {
    deleteAll(left, right);
}

void BinaryExp::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    left->traverse(walkers, this);
    right->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

list<Tac*> BinaryExp::translate(shared_ptr<TacOperand> place) {
    list<Tac*> code;
    if (opt == OPT_AND || opt == OPT_OR ||
        opt == OPT_LT || opt == OPT_LE ||
        opt == OPT_GT || opt == OPT_GE ||
        opt == OPT_NE || opt == OPT_EQ
    ) {
        return translateCondExp(this, place);
    } else {
        auto t1 = makeTacOp<VariableOperand>(scope->createPlace());
        auto t2 = makeTacOp<VariableOperand>(scope->createPlace());
        auto code1 = left->translate(t1);
        auto code2 = right->translate(t2);
        ArithTac *code3;
        switch (opt) {
        case OPT_PLUS:
            code3 = new AddTac(place, t1, t2);
            break;
        case OPT_MINUS:
            code3 = new SubTac(place, t1, t2);
            break;
        case OPT_MUL:
            code3 = new MulTac(place, t1, t2);
            break;
        case OPT_DIV:
            code3 = new DivTac(place, t1, t2);
            break;
        default:
            for (auto tac: code1) delete tac;
            for (auto tac: code2) delete tac;
            throw runtime_error("Invalid binary operator");
        }
        code1.splice(code1.end(), code2);
        code1.push_back(code3);
        return code1;
    }
}


AssignExp::AssignExp(Exp * left, Exp * right): left(left), right(right) {
    if (hasNull(left, right)) {
        deleteAll(left, right);
        throw invalid_argument("left/right cannot be null");
    }
}

AssignExp::~AssignExp() {
    deleteAll(left, right);
}

void AssignExp::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    left->traverse(walkers, this);
    right->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

list<Tac*> AssignExp::translate(shared_ptr<TacOperand> place) {
    auto lvalue = dynamic_cast<const IdExp*>(left);
    if (lvalue == nullptr) {
        throw runtime_error("assignment to non-IdExp has not been implemented");
    }
    auto variable = makeTacOp<VariableOperand>(scope->getId(lvalue->identifier));
    auto tp = makeTacOp<VariableOperand>(scope->createPlace());
    auto code = right->translate(tp);
    code.push_back(new AssignTac(variable, tp));
    code.push_back(new AssignTac(place, variable));
    return code;
}


CallExp::CallExp(const string& identifier, const list<Exp*>& arguments):
    identifier(identifier), arguments(arguments.begin(), arguments.end())
{
    if (hasNull(this->arguments)) {
        deleteAll(this->arguments);
        throw invalid_argument("argument cannot be null");
    }
}

CallExp::~CallExp() {
    deleteAll(arguments);
}

// TODO: enforce argument number for built-in I/O functions during semantic analysis
list<Tac*> CallExp::translate(shared_ptr<TacOperand> place) {
    list<Tac*> code;
    if (identifier == "read") {
        code.push_back(new ReadTac(place));
    } else if (identifier == "write") {
        auto tp = makeTacOp<VariableOperand>(scope->createPlace());
        code.splice(code.end(), arguments[0]->translate(tp));
        code.push_back(new WriteTac(tp));
    } else {
        vector<shared_ptr<TacOperand>> argPlaces;
        // left-to-right evaluation
        for (auto arg: arguments) {
            auto argPlace = makeTacOp<VariableOperand>(scope->createPlace());
            code.splice(code.end(), arg->translate(argPlace));
            argPlaces.push_back(argPlace);
        }
        // push args into stack from right to left
        for (auto argPlace = argPlaces.rbegin(); argPlace != argPlaces.rend(); ++argPlace) {
            code.push_back(new ArgTac(*argPlace));
        }
        code.push_back(new CallTac(place, identifier));
    }
    return code;
}


void CallExp::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    for (auto arg: arguments) arg->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}



// ------------------------- statements -----------------------

ExpStmt::ExpStmt(Exp * expression): expression(expression) {
    if (expression == nullptr) throw invalid_argument("expression cannot be null");
}

void ExpStmt::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    expression->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

list<Tac*> ExpStmt::translate() {
    auto tp = makeTacOp<VariableOperand>(scope->createPlace());
    return expression->translate(tp);
}


void ReturnStmt::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    if (argument) argument->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

list<Tac*> ReturnStmt::translate() {
    auto tp = makeTacOp<VariableOperand>(scope->createPlace());
    auto code = argument->translate(tp);
    code.push_back(new ReturnTac(tp));
    return code;
}


IfStmt::IfStmt(Exp * test, Stmt * consequent, Stmt * alternate):
    test(test), consequent(consequent), alternate(alternate)
{
    if (hasNull(test, consequent)) {
        deleteAll(test, consequent, alternate);
        throw invalid_argument("test/consequent cannot be null");
    }
}

void IfStmt::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    test->traverse(walkers, this);
    consequent->traverse(walkers, this);
    if (alternate) alternate->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

list<Tac*> IfStmt::translate() {
    LabelTac *label1, *label2, *label3;
    label1 = new LabelTac(scope->createLabel());
    label2 = new LabelTac(scope->createLabel());
    list<Tac*> code = translateCondExp(test, label1, label2);
    code.push_back(label1);
    code.splice(code.end(), consequent->translate());
    if (alternate != nullptr) {
        label3 = new LabelTac(scope->createLabel());
        code.push_back(new GotoTac(label3->no));
    }
    code.push_back(label2);
    if (alternate != nullptr) {
        code.splice(code.end(), alternate->translate());
        code.push_back(label3);
    }
    return code;
}


WhileStmt::WhileStmt(Exp * test, Stmt * body): test(test), body(body) {
    if (hasNull(test, body)) {
        deleteAll(test, body);
        throw invalid_argument("test/body cannot be null");
    }
}

void WhileStmt::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    test->traverse(walkers, this);
    body->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

list<Tac*> WhileStmt::translate() {
    auto label1 = new LabelTac(scope->createLabel());
    auto label2 = new LabelTac(scope->createLabel());
    auto label3 = new LabelTac(scope->createLabel());
    list<Tac*> code { label1 };
    code.splice(code.end(), translateCondExp(test, label2, label3));
    code.push_back(label2);
    code.splice(code.end(), body->translate());
    code.push_back(new GotoTac(label1->no));
    code.push_back(label3);
    return code;
}


ForStmt::ForStmt(Exp * init, Exp * test, Exp * update, Stmt * body):
    init(init), test(test), update(update), body(body)
{
    if (body == nullptr) {
        deleteAll(init, test, update);
        throw invalid_argument("body cannot be null");
    }
}

void ForStmt::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    if (init) init->traverse(walkers, this);
    if (test) test->traverse(walkers, this);
    if (update) update->traverse(walkers, this);
    body->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

list<Tac*> ForStmt::translate() {
    // TODO
    throw runtime_error("not implemented");
}


void CompoundStmt::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    for (auto def: definitions) def->traverse(walkers, this);
    for (auto stmt: body) stmt->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

list<Tac*> CompoundStmt::translate() {
    list<Tac*> code;
    for (auto stmt: body) {
        code.splice(code.end(), stmt->translate());
    }
    return code;
}

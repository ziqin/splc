#include <cctype>
#include <exception>
#include <typeinfo>
#include <utility>
#include "ast.hpp"
#include "syntax.hpp"
#include "type.hpp"

using namespace ast;
using namespace std;
using namespace ir;


template <typename T>
inline static void execEnter(initializer_list<Visitor*> visitors, T *self, Node *parent) {
    for (Visitor *visitor: visitors) {
        visitor->enter(self, parent);
    }
}

template <typename T>
inline static void execLeave(initializer_list<Visitor*> visitors, T *self, Node *parent) {
    for (Visitor *visitor: visitors) {
        visitor->leave(self, parent);
    }
}


Node::Node() {
    static int gid = 0;
    nodeId = gid++;
}

void Node::setLocation(const YYLTYPE * location) {
    loc.start.line = location->first_line;
    loc.start.column = location->first_column;
    loc.end.line = location->last_line;
    loc.end.column = location->last_column;
}

void Node::visit(Visitor *visitor) {
    visitor->visit(this);
}

void Node::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    execLeave(visitors, this, parent);
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

void Program::visit(Visitor *visitor) {
    visitor->visit(this);
}

void Program::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    for (auto def: extDefs) def->traverse(visitors, this);
    execLeave(visitors, this, parent);
}

void Program::traverse(initializer_list<Visitor*> visitors) {
    traverse(visitors, nullptr);
}

// ----------------------------- declaration/definition --------------------------------

void VarDec::visit(Visitor *visitor) {
    visitor->visit(this);
}

void VarDec::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    execLeave(visitors, this, parent);
}


ArrDec::ArrDec(const VarDec& declarator, int dim): VarDec(declarator.identifier) {
    try {
        auto& prefix = dynamic_cast<const ArrDec&>(declarator);
        dimensions = prefix.dimensions;
    } catch (const bad_cast& e) {}
    dimensions.push_back(dim);
}

void ArrDec::visit(Visitor *visitor) {
    visitor->visit(this);
}

void ArrDec::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    execLeave(visitors, this, parent);
}


Dec::Dec(VarDec * declarator, Exp * init): declarator(declarator), init(init) {
    if (declarator == nullptr) throw invalid_argument("declarator cannot be null");
}

Dec::~Dec() {
    deleteAll(declarator, init);
}

void Dec::visit(Visitor *visitor) {
    visitor->visit(this);
}

void Dec::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    declarator->traverse(visitors, this);
    if (init) init->traverse(visitors, this);
    execLeave(visitors, this, parent);
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

void Def::visit(Visitor *visitor) {
    visitor->visit(this);
}

void Def::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    specifier->traverse(visitors, this);
    for (auto dec: declarations) dec->traverse(visitors, this);
    execLeave(visitors, this, parent);
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

void ParamDec::visit(Visitor *visitor) {
    visitor->visit(this);
}

void ParamDec::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    specifier->traverse(visitors, this);
    declarator->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


FunDec::FunDec(string identifier, const list<ParamDec*>& varList):
    identifier(std::move(identifier)), parameters(varList.begin(), varList.end())
{
    if (hasNull(this->parameters)) {
        deleteAll(this->parameters);
        throw invalid_argument("parameters cannot be null");
    }
}

FunDec::~FunDec() {
    deleteAll(parameters);
}

void FunDec::visit(Visitor *visitor) {
    visitor->visit(this);
}

void FunDec::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    for (auto para: parameters) para->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


void Specifier::visit(Visitor *visitor) {
    visitor->visit(this);
}

void Specifier::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    execLeave(visitors, this, parent);
}


PrimitiveSpecifier::PrimitiveSpecifier(const string& typeName) {
    if (typeName == "char") {
        primitive = smt::Primitive::CHAR;
        type.set(new smt::PrimitiveType(smt::Primitive::CHAR));
    } else if (typeName == "int") {
        primitive = smt::Primitive::INT;
        type.set(new smt::PrimitiveType(smt::Primitive::INT));
    } else if (typeName == "float") {
        primitive = smt::Primitive::FLOAT;
        type.set(new smt::PrimitiveType(smt::Primitive::FLOAT));
    } else {
        throw invalid_argument("illegal primitive type");
    }
}

void PrimitiveSpecifier::visit(Visitor *visitor) {
    visitor->visit(this);
}

void PrimitiveSpecifier::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    execLeave(visitors, this, parent);
}


StructSpecifier::StructSpecifier(string identifier, const list<Def*>& defList):
    identifier(std::move(identifier)), definitions(defList.begin(), defList.end())
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

void StructSpecifier::visit(Visitor *visitor) {
    visitor->visit(this);
}

void StructSpecifier::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    for (auto def: definitions) def->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


void ExtDef::visit(Visitor *visitor) {
    visitor->visit(this);
}

void ExtDef::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    execLeave(visitors, this, parent);
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

void ExtVarDef::visit(Visitor *visitor) {
    visitor->visit(this);
}

void ExtVarDef::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    specifier->traverse(visitors, this);
    for (auto dec: varDecs) dec->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


StructDef::StructDef(Specifier * specifier):
    specifier(dynamic_cast<StructSpecifier*>(specifier))
{
    if (specifier == nullptr) throw invalid_argument("type of `specifier` should be StructSpecifier");
}

StructDef::~StructDef() {
    delete specifier;
}

void StructDef::visit(Visitor *visitor) {
    visitor->visit(this);
}

void StructDef::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    specifier->traverse(visitors, this);
    execLeave(visitors, this, parent);
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

void FunDef::visit(Visitor *visitor) {
    visitor->visit(this);
}

void FunDef::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    specifier->traverse(visitors, this);
    declarator->traverse(visitors, this);
    body->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


// ----------------------- expressions ---------------------------

void Exp::visit(Visitor *visitor) {
    visitor->visit(this);
}

void Exp::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    execLeave(visitors, this, parent);
}


LiteralExp::LiteralExp(char val):
    Exp(Shared<smt::Type>(new smt::PrimitiveType(smt::Primitive::CHAR))),
    charVal(val) {}

LiteralExp::LiteralExp(int val):
    Exp(Shared<smt::Type>(new smt::PrimitiveType(smt::Primitive::INT))),
    intVal(val) {}

LiteralExp::LiteralExp(double val):
    Exp(Shared<smt::Type>(new smt::PrimitiveType(smt::Primitive::FLOAT))),
    floatVal(val) {}

void LiteralExp::visit(Visitor *visitor) {
    visitor->visit(this);
}

void LiteralExp::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    execLeave(visitors, this, parent);
}


IdExp::IdExp(const string& identifier): identifier(identifier) {
    auto itr = identifier.begin();
    if (itr == identifier.end()) throw invalid_argument("identifier cannot be empty");
    if (!isalpha(*itr) && *itr != '_') throw invalid_argument("illegal identifier");
    for (++itr; itr != identifier.end(); ++itr) {
        if (!isalnum(*itr) && *itr != '_') throw invalid_argument("illegal identifier");
    }
}

void IdExp::visit(Visitor *visitor) {
    visitor->visit(this);
}

void IdExp::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    execLeave(visitors, this, parent);
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

void ArrayExp::visit(Visitor *visitor) {
    visitor->visit(this);
}

void ArrayExp::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    subject->traverse(visitors, this);
    index->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


MemberExp::MemberExp(Exp *subject, string member):
    subject(subject), member(std::move(member))
{
    if (subject == nullptr) throw invalid_argument("subject cannot be null");
}

MemberExp::~MemberExp() {
    delete subject;
}

void MemberExp::visit(Visitor *visitor) {
    visitor->visit(this);
}

void MemberExp::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    subject->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


UnaryExp::UnaryExp(Operator opt, Exp * argument):
    opt(opt), argument(argument)
{
    if (opt != Operator::MINUS && opt != Operator::NOT && opt != Operator::PLUS) {
        delete argument;
        throw invalid_argument("illegal operator for unary expression");
    }
}

UnaryExp::~UnaryExp() {
    delete argument;
}

void UnaryExp::visit(Visitor *visitor) {
    visitor->visit(this);
}

void UnaryExp::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    argument->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


BinaryExp::BinaryExp(Exp * left, Operator opt, Exp * right):
    opt(opt), left(left), right(right)
{
    if (opt == Operator::NOT) {
        deleteAll(this->left, this->right);
        throw invalid_argument("illegal operator for binary expression");
    }
}

BinaryExp::~BinaryExp() {
    deleteAll(left, right);
}

void BinaryExp::visit(Visitor *visitor) {
    visitor->visit(this);
}

void BinaryExp::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    left->traverse(visitors, this);
    right->traverse(visitors, this);
    execLeave(visitors, this, parent);
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

void AssignExp::visit(Visitor *visitor) {
    visitor->visit(this);
}

void AssignExp::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    left->traverse(visitors, this);
    right->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


CallExp::CallExp(string identifier, const list<Exp*>& arguments):
    identifier(std::move(identifier)), arguments(arguments.begin(), arguments.end())
{
    if (hasNull(this->arguments)) {
        deleteAll(this->arguments);
        throw invalid_argument("argument cannot be null");
    }
}

CallExp::~CallExp() {
    deleteAll(arguments);
}

void CallExp::visit(Visitor *visitor) {
    visitor->visit(this);
}

void CallExp::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    for (auto arg: arguments) arg->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


// ------------------------- statements -----------------------

void Stmt::visit(Visitor *visitor) {
    visitor->visit(this);
}

void Stmt::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    execLeave(visitors, this, parent);
}


ExpStmt::ExpStmt(Exp * expression): expression(expression) {
    if (expression == nullptr) throw invalid_argument("expression cannot be null");
}

void ExpStmt::visit(Visitor *visitor) {
    visitor->visit(this);
}

void ExpStmt::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    expression->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


void ReturnStmt::visit(Visitor *visitor) {
    visitor->visit(this);
}

void ReturnStmt::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    if (argument) argument->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


IfStmt::IfStmt(Exp * test, Stmt * consequent, Stmt * alternate):
    test(test), consequent(consequent), alternate(alternate)
{
    if (hasNull(test, consequent)) {
        deleteAll(test, consequent, alternate);
        throw invalid_argument("test/consequent cannot be null");
    }
}

void IfStmt::visit(Visitor *visitor) {
    visitor->visit(this);
}

void IfStmt::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    test->traverse(visitors, this);
    consequent->traverse(visitors, this);
    if (alternate) alternate->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


WhileStmt::WhileStmt(Exp * test, Stmt * body): test(test), body(body) {
    if (hasNull(test, body)) {
        deleteAll(test, body);
        throw invalid_argument("test/body cannot be null");
    }
}

void WhileStmt::visit(Visitor *visitor) {
    visitor->visit(this);
}

void WhileStmt::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    test->traverse(visitors, this);
    body->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


ForStmt::ForStmt(Exp * init, Exp * test, Exp * update, Stmt * body):
    init(init), test(test), update(update), body(body)
{
    if (body == nullptr) {
        deleteAll(init, test, update);
        throw invalid_argument("body cannot be null");
    }
}

void ForStmt::visit(Visitor *visitor) {
    visitor->visit(this);
}

void ForStmt::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    if (init) init->traverse(visitors, this);
    if (test) test->traverse(visitors, this);
    if (update) update->traverse(visitors, this);
    body->traverse(visitors, this);
    execLeave(visitors, this, parent);
}


void CompoundStmt::visit(Visitor *visitor) {
    visitor->visit(this);
}

void CompoundStmt::traverse(initializer_list<Visitor*> visitors, Node *parent) {
    execEnter(visitors, this, parent);
    for (auto def: definitions) def->traverse(visitors, this);
    for (auto stmt: body) stmt->traverse(visitors, this);
    execLeave(visitors, this, parent);
}

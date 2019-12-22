#include <cctype>
#include <exception>
#include <typeinfo>
#include "ast.hpp"
#include "ast_walker.hpp"
#include "syntax.tab.h"

using namespace AST;
using namespace std;


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
    id = gid++;
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


FunDec::FunDec(const string& id, const list<ParamDec*>& varList):
    identifier(id), parameters(varList.begin(), varList.end())
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


StructSpecifier::StructSpecifier(const string& id, const list<Def*>& defList):
    identifier(id), definitions(defList.begin(), defList.end())
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

LiteralExp::LiteralExp(char val):
    Exp(Shared<smt::Type>(new smt::PrimitiveType(smt::TYPE_CHAR))),
    charVal(val) {}

LiteralExp::LiteralExp(int val):
    Exp(Shared<smt::Type>(new smt::PrimitiveType(smt::TYPE_INT))),
    intVal(val) {}

LiteralExp::LiteralExp(double val):
    Exp(Shared<smt::Type>(new smt::PrimitiveType(smt::TYPE_FLOAT))),
    floatVal(val) {}


IdExp::IdExp(const std::string& id): identifier(id) {
    auto itr = id.begin();
    if (itr == id.end()) throw invalid_argument("id cannot be empty");
    if (!isalpha(*itr) && *itr != '_') throw invalid_argument("illegal id");
    for (++itr; itr != id.end(); ++itr) {
        if (!isalnum(*itr) && *itr != '_') throw invalid_argument("illegal id");
    }
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


CallExp::CallExp(const std::string& id, const list<Exp*>& arguments):
    identifier(id), arguments(arguments.begin(), arguments.end())
{
    if (hasNull(this->arguments)) {
        deleteAll(this->arguments);
        throw invalid_argument("argument cannot be null");
    }
}

CallExp::~CallExp() {
    deleteAll(arguments);
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


void ReturnStmt::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    if (argument) argument->traverse(walkers, this);
    execPostHook(walkers, this, parent);
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


void CompoundStmt::traverse(const vector<Walker*>& walkers, Node * parent) {
    execPreHook(walkers, this, parent);
    for (auto def: definitions) def->traverse(walkers, this);
    for (auto stmt: body) stmt->traverse(walkers, this);
    execPostHook(walkers, this, parent);
}

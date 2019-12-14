#include <cctype>
#include <exception>
#include "ast.hpp"
#include "utils.hpp"

using namespace AST;
using namespace std;


// ----------------------------- misc --------------------------------

void Node::setScope(shared_ptr<SymbolTable> scope) {
    this->scope = scope;
}

ArrDec::ArrDec(const VarDec& declarator, unsigned dim): VarDec(declarator.identifier) {
    try {
        auto& prefix = dynamic_cast<const ArrDec&>(declarator);
        dimensions = prefix.dimensions;
    } catch (const bad_cast& e) {}
    dimensions.push_back(dim);
}


Dec::Dec(VarDec * declarator, Exp * init): declarator(declarator), init(init) {
    if (hasNull(this->declarator, this->init)) {
        deleteAll(this->declarator, this->init);
        throw invalid_argument("declarator/init cannot be null");
    }
}

Dec::~Dec() {
    deleteAll(declarator, init);
}

void Dec::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    declarator->setScope(scope);
    init->setScope(scope);
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

void Def::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    specifier->setScope(scope);
    for (auto dec: declarations) dec->setScope(scope);
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

void ParamDec::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    specifier->setScope(scope);
    declarator->setScope(scope);
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

void FunDec::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    for (auto para: parameters) para->setScope(scope);
}

PrimitiveSpecifier::PrimitiveSpecifier(const string& type) {
    if (type == "char") primitive = TYPE_CHAR;
    else if (type == "int") primitive = TYPE_INT;
    else if (type == "float") primitive = TYPE_FLOAT;
    else throw invalid_argument("illegal primitive type");
}


StructSpecifier::StructSpecifier(const string& id, const list<Def*>& defList):
    identifier(id), definitions(defList.begin(), defList.end())
{
    if (hasNull(this->definitions)) {
        deleteAll(this->definitions);
        throw invalid_argument("definitions cannot be null");
    }
}

StructSpecifier::~StructSpecifier() {
    deleteAll(definitions);
}

void StructSpecifier::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    for (auto def: definitions) def->setScope(scope);
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

void ExtVarDef::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    specifier->setScope(scope);
    for (auto dec: varDecs) dec->setScope(scope);
}

StructDef::StructDef(Specifier * specifier):
    specifier(dynamic_cast<StructSpecifier*>(specifier))
{
    if (specifier == nullptr) throw invalid_argument("type of `specifier` should be StructSpecifier");
}

StructDef::~StructDef() {
    delete specifier;
}

void StructDef::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    specifier->setScope(scope);
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

void FunDef::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = make_shared<SymbolTable>(scope);
    specifier->setScope(scope);
    declarator->setScope(this->scope);
    body->setScope(this->scope);
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

void Program::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    for (auto def: extDefs) def->setScope(scope);
}


// ----------------------- expressions ---------------------------

LiteralExp::LiteralExp(char val):
    Exp(shared_ptr<Type>(new PrimitiveType(TYPE_CHAR))),
    charVal(val) {}

LiteralExp::LiteralExp(int val):
    Exp(shared_ptr<Type>(new PrimitiveType(TYPE_INT))),
    intVal(val) {}

LiteralExp::LiteralExp(double val):
    Exp(shared_ptr<Type>(new PrimitiveType(TYPE_FLOAT))),
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

void ArrayExp::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    subject->setScope(scope);
    index->setScope(scope);
}

MemberExp::MemberExp(Exp * subject, Exp * member):
    subject(subject), member(dynamic_cast<IdExp*>(member))
{
    if (this->subject == nullptr) {
        deleteAll(this->subject, this->member);
        throw invalid_argument("subject cannot be null");
    }
    if (this->member == nullptr) {
        deleteAll(this->subject, this->member);
        throw invalid_argument("type of `member` should be IdExp");
    }
}

MemberExp::~MemberExp() {
    deleteAll(subject, member);
}

void MemberExp::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    subject->setScope(scope);
    member->setScope(scope);
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

void UnaryExp::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    argument->setScope(scope);
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

void BinaryExp::setScope(shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    left->setScope(scope);
    right->setScope(scope);
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

void AssignExp::setScope(shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    left->setScope(scope);
    right->setScope(scope);
}

CallExp::CallExp(Exp * callee, initializer_list<Exp*> arguments):
    callee(dynamic_cast<IdExp*>(callee)), arguments(arguments)
{
    if (this->callee == nullptr) {
        deleteAll(this->callee, this->arguments);
        throw invalid_argument("type of `callee` should be IdExp");
    }
    if (hasNull(this->arguments)) {
        deleteAll(this->arguments);
        throw invalid_argument("argument cannot be null");
    }
}

CallExp::~CallExp() {
    deleteAll(callee, arguments);
}

void CallExp::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    callee->setScope(scope);
    for (auto arg: arguments) arg->setScope(scope);
}


// ------------------------- statements -----------------------

ExpStmt::ExpStmt(Exp * expression): expression(expression) {
    if (expression == nullptr) throw invalid_argument("expression cannot be null");
}

void ExpStmt::setScope(shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    expression->scope = scope;
}

void ReturnStmt::setScope(shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    argument->setScope(scope);
}

IfStmt::IfStmt(Exp * test, Stmt * consequent, Stmt * alternate):
    test(test), consequent(consequent), alternate(alternate)
{
    if (hasNull(test, consequent)) {
        deleteAll(test, consequent, alternate);
        throw invalid_argument("test/consequent cannot be null");
    }
}

void IfStmt::setScope(shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    test->setScope(scope);
    consequent->setScope(scope);
    if (alternate) alternate->setScope(scope);
}

WhileStmt::WhileStmt(Exp * test, Stmt * body): test(test), body(body) {
    if (hasNull(test, body)) {
        deleteAll(test, body);
        throw invalid_argument("test/body cannot be null");
    }
}

void WhileStmt::setScope(shared_ptr<SymbolTable> scope) {
    this->scope = scope;
    test->setScope(scope);
    body->setScope(scope);
}

ForStmt::ForStmt(Exp * init, Exp * test, Exp * update, Stmt * body):
    init(init), test(test), update(update), body(body)
{
    if (body == nullptr) {
        deleteAll(init, test, update);
        throw invalid_argument("body cannot be null");
    }
}

void ForStmt::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = make_shared<SymbolTable>(scope);
    if (init != nullptr) init->setScope(this->scope);
    if (test != nullptr) test->setScope(this->scope);
    if (update != nullptr) update->setScope(this->scope);
    body->setScope(this->scope);
}

void CompoundStmt::setScope(std::shared_ptr<SymbolTable> scope) {
    this->scope = make_shared<SymbolTable>(scope);
    for (auto def: definitions) def->setScope(this->scope);
    for (auto stmt: body) stmt->setScope(this->scope);
}

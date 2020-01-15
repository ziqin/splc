#include <typeinfo>
#include "semantic.hpp"
#include "type.hpp"

using namespace ast;
using namespace smt;
using namespace std;


// TODO: refactor type resolution for structure and array
// Consider Future pattern

vector<SemanticErrRecord> smt::analyzeSemantic(Program *ast) {
    vector<SemanticErrRecord> semanticErrs;
    auto scopeSetter = make_unique<ScopeSetter>();
    auto structInit = make_unique<StructInitializer>(semanticErrs); 
    auto symbolSetter = make_unique<SymbolSetter>(semanticErrs);
    auto typeSynthesizer = make_unique<TypeSynthesizer>(semanticErrs);

    // order matters!
    ast->traverse({ scopeSetter.get(), structInit.get() });
    ast->traverse({ symbolSetter.get(), typeSynthesizer.get() });

    return semanticErrs;
}


void ScopeSetter::defaultEnter(Node *self, Node *parent) {
    if (self->scope == nullptr) { // scope has not been specified by its parent
        self->scope = parent->scope;
    }
}

void ScopeSetter::enter(Program *self, Node *parent) {
    self->scope = make_shared<SymbolTable>();
}

void ScopeSetter::enter(FunDef *self, Node *parent) {
    self->scope = parent->scope;
    auto inFuncScope = make_shared<SymbolTable>(self->scope);
    for (ParamDec * para: self->declarator->parameters) {
        para->scope = inFuncScope;
    }
    self->body->scope = inFuncScope;
    // TODO: check whether var definitions in body block are allowed to overwrite function parameters in C language
}

void ScopeSetter::enter(ForStmt *self, Node *parent) {
    self->scope = make_shared<SymbolTable>(parent->scope);
}

void ScopeSetter::enter(CompoundStmt *self, Node *parent) {
    if (self->scope == nullptr) {
        self->scope = make_shared<SymbolTable>(parent->scope);
    }
}


void SemanticAnalyzer::report(SemanticErr errType, Node *cause, const std::string& msg) {
    errs.emplace_back(errType, cause, msg);
    nodesWithErr.insert(cause->nodeId);
}

void SemanticAnalyzer::report(Node *cause) {
    nodesWithErr.insert(cause->nodeId);
}

bool SemanticAnalyzer::hasErr(Node *node) const {
    return nodesWithErr.find(node->nodeId) != nodesWithErr.end();
}


void StructInitializer::enter(StructDef *self, Node *parent) {
    const string& name = self->specifier->identifier;
    if (this->structures.find(name) != this->structures.end()) {
        this->report(SemanticErr::TYPE15, self, "redefine the same structure type");
    } else {
        this->structures[name] = self->specifier->type;
    }
}

void StructInitializer::leave(StructDef *self, Node *parent) {
    if (!this->hasErr(self)) {
        Shared<Type> type = this->structures[self->specifier->identifier];
        auto structType = new StructType;
        for (Def *def: self->specifier->definitions) {
            // Shared<Type> fieldType = def->specifier->type;
            for (Dec *dec: def->declarations) {
                Shared<Type> fieldType = def->specifier->type;
                auto arrField = dynamic_cast<const ArrDec*>(dec->declarator);  // deduplicate
                if (arrField != nullptr) {
                    for (auto dim = arrField->dimensions.rbegin(); dim != arrField->dimensions.rend(); ++dim) {
                        fieldType = makeType<ArrayType>(fieldType, *dim);
                    }
                }
                structType->fields.emplace_back(fieldType, dec->declarator->identifier);
            }
        }
        type.set(structType);
    }
}

void StructInitializer::enter(StructSpecifier *self, Node *parent) {
    auto typeItr = this->structures.find(self->identifier);
    if (typeItr == this->structures.end()) {
        this->report(SemanticErr::TYPE0, self, "struct undefined");
    } else {
        self->type = typeItr->second;
    }
}


void SymbolSetter::enter(Program *self, Node *parent) {
    auto readType = makeType<FunctionType>(makeType<PrimitiveType>(Primitive::INT));
    auto writeType = makeType<FunctionType>(makeType<PrimitiveType>(Primitive::INT), vector<Shared<Type>>{
        makeType<PrimitiveType>(Primitive::INT)
    });
    self->scope->setType("read", readType);
    self->scope->setType("write", writeType);
}

void SymbolSetter::enter(ExtVarDef *self, Node *parent) {
    for (VarDec *var: self->varDecs) {
        this->typeRefs[var->nodeId] = self->specifier->type;
    }
}

void SymbolSetter::enter(ParamDec *self, Node *parent) {
    this->typeRefs[self->declarator->nodeId] = self->specifier->type;
}

void SymbolSetter::enter(FunDef *self, Node *parent) {
    this->typeRefs[self->declarator->nodeId] = self->specifier->type;
}

void SymbolSetter::enter(FunDec *self, Node *parent) {
    if (self->scope->canOverwrite(self->identifier)) {
        auto *type = new FunctionType(this->typeRefs[self->nodeId]);
        for (ParamDec *para: self->parameters) {
            Shared<Type> paraType = para->specifier->type;
            auto arrPara = dynamic_cast<const ArrDec*>(para->declarator);
            if (arrPara != nullptr) {   // TODO: deduplicate
                for (auto dim = arrPara->dimensions.rbegin(); dim != arrPara->dimensions.rend(); ++dim) {
                    paraType = makeType<ArrayType>(paraType, *dim);
                }
            }
            type->parameters.push_back(paraType);
        }
        self->scope->setType(self->identifier, Shared<Type>(type));
    } else {
        this->report(SemanticErr::TYPE4, self, "function `" + self->identifier +"' is redefined");
    }
}

void SymbolSetter::enter(Def *self, Node *parent) {
    for (Dec *dec: self->declarations) {
        this->typeRefs[dec->nodeId] = self->specifier->type;
    }
}

void SymbolSetter::enter(Dec *self, Node *parent) {
    this->typeRefs[self->declarator->nodeId] = this->typeRefs[self->nodeId];
}

void SymbolSetter::leave(VarDec *self, Node *parent) {
    if (!self->scope->canOverwrite(self->identifier)) {
        this->report(SemanticErr::TYPE3, self, "variable `" + self->identifier + "' is redefined in the same scope");
    }
    self->scope->setType(self->identifier, this->typeRefs[self->nodeId]);
}

void SymbolSetter::leave(ArrDec *self, Node *parent) {
    Shared<Type> type = this->typeRefs[self->nodeId];
    for (auto dim = self->dimensions.rbegin(); dim != self->dimensions.rend(); ++dim) {
        type = makeType<ArrayType>(type, *dim);
    }
    self->scope->setType(self->identifier, type);
}


void TypeSynthesizer::leave(IdExp *self, Node *parent) {
    optional<Shared<Type>> defined = self->scope->getType(self->identifier);
    if (defined) {
        self->type = defined.value();
    } else {
        this->report(SemanticErr::TYPE1, self, "variable " + self->identifier + " is used without definition");
    }
}

void TypeSynthesizer::leave(CallExp *self, Node *parent) {
    for (auto arg: self->arguments) {
        if (this->hasErr(arg)) {
            this->report(self);
            return;
        }
    }
    optional<Shared<Type>> defined = self->scope->getType(self->identifier);
    if (defined) {
        try {
            const FunctionType& funcType = as<FunctionType>(defined.value());
            bool argMatch = funcType.parameters.size() == self->arguments.size();
            if (argMatch) {
                for (size_t i = 0; i < funcType.parameters.size(); ++i) {
                    argMatch = argMatch && *funcType.parameters[i] == *(self->arguments[i]->type);
                }
            }
            if (argMatch) {
                self->type = funcType.returned;
            } else {
                this->report(SemanticErr::TYPE9, self, "the arguments of function `" + self->identifier + "' mismatch the declared parameters");
            }
        } catch (const exception& e) {
            this->report(SemanticErr::TYPE11, self, "applying function invocation operator on non-function names");
        }
    } else {
        this->report(SemanticErr::TYPE2, self, "function is invoked without definition");
    }
}

// TODO: check type compatibility instead of equivalence
void TypeSynthesizer::leave(AssignExp *self, Node *parent) {
    if (this->hasErr(self->left) || this->hasErr(self->right)) {
        this->report(self);
        return;
    }
    if (typeid(*self->left) != typeid(IdExp) &&
        typeid(*self->left) != typeid(ArrayExp) &&
        typeid(*self->left) != typeid(MemberExp)
    )   this->report(SemanticErr::TYPE6, self, "rvalue on the left side of assignment operator");
    if (self->left->type.value() != self->right->type.value())
        this->report(SemanticErr::TYPE5, self, "unmatched types on both sides of assignment operator");
    self->type = self->left->type;
}

void TypeSynthesizer::leave(Dec *self, Node *parent) {
    if (this->hasErr(self->declarator) || (self->init != nullptr && this->hasErr(self->init))) {
        this->report(self);
        return;
    }
    if (self->init != nullptr &&
        self->scope->getType(self->declarator->identifier).value().value() != self->init->type.value()
    ) {
        this->report(SemanticErr::TYPE5, self, "unmatched types on both sides of assignment operator");
    }
}

void TypeSynthesizer::leave(UnaryExp *self, Node *parent) {
    if (this->hasErr(self->argument)) {
        this->report(self);
        return;
    }
    try {
        const PrimitiveType& type = as<PrimitiveType>(self->argument->type);
        if (type == Primitive::CHAR) this->report(SemanticErr::TYPE7, self, "unmatched operand");
        else self->type = self->argument->type;
    } catch (const exception& e) {
        this->report(SemanticErr::TYPE7, self, "unmatched operand");
    }
}

void TypeSynthesizer::leave(BinaryExp *self, Node *parent) {
    if (this->hasErr(self->left) || this->hasErr(self->right)) {
        this->report(self);
        return;
    }
    Shared<Type> left = self->left->type, right = self->right->type;
    if (typeid(left.value()) != typeid(PrimitiveType) ||
        typeid(right.value()) != typeid(PrimitiveType) ||
        *left != *right
    ) {
        this->report(SemanticErr::TYPE7, self, "unmatched operands");
    } else {
        self->type = left;
    }
}

void TypeSynthesizer::leave(MemberExp *self, Node *parent) {
    if (this->hasErr(self->subject)) {
        this->report(self);
        return;
    }
    Shared<Type> type = self->subject->type;
    try {
        const StructType& structType = as<StructType>(type);
        Shared<Type> fieldType = structType.getFieldType(self->member);
        if (fieldType == nullptr) {
            this->report(SemanticErr::TYPE14, self, "accessing an undefined structure member `" + self->member + "'");
        } else {
            self->type = fieldType;
        }
    } catch (const exception& e) {
        this->report(SemanticErr::TYPE13, self, "accessing member of non-structure variable");
    }
}

void TypeSynthesizer::leave(ArrayExp *self, Node *parent) {
    if (this->hasErr(self->subject) || this->hasErr(self->index)) {
        this->report(self);
        return;
    }
    const auto *arrayType = dynamic_cast<const ArrayType*>(self->subject->type.get());
    const auto *indexType = dynamic_cast<const PrimitiveType*>(self->index->type.get());
    if (arrayType == nullptr) {
        this->report(SemanticErr::TYPE10, self, "applying indexing operator on non-array type variables");
    }
    if (indexType == nullptr || !(*indexType == Primitive::INT)) {
        this->report(SemanticErr::TYPE12, self, "array indexing with non-integer type expression");
    } else if (arrayType != nullptr) {
        self->type = arrayType->baseType;
    }
}

void TypeSynthesizer::enter(FunDef *self, Node *parent) {
    this->funcReturnTypes[self->body->nodeId] = self->specifier->type;
}

void TypeSynthesizer::enter(CompoundStmt *self, Node *parent) {
    auto returnType = this->funcReturnTypes[self->nodeId];
    for (auto stmt: self->body) {
        this->funcReturnTypes[stmt->nodeId] = returnType;
    }
}

void TypeSynthesizer::enter(IfStmt *self, Node *parent) {
    this->funcReturnTypes[self->consequent->nodeId] = this->funcReturnTypes[self->nodeId];
    if (self->alternate != nullptr) {
        this->funcReturnTypes[self->alternate->nodeId] = this->funcReturnTypes[self->nodeId];
    }
}

void TypeSynthesizer::enter(WhileStmt *self, Node *parent) {
    this->funcReturnTypes[self->body->nodeId] = this->funcReturnTypes[self->nodeId];
}

void TypeSynthesizer::enter(ForStmt *self, Node *parent) {
    this->funcReturnTypes[self->body->nodeId] = this->funcReturnTypes[self->nodeId];
}

void TypeSynthesizer::leave(ReturnStmt *self, Node *parent) {
    if (this->hasErr(self->argument)) {
        this->report(self);
        return;
    }
    if (this->funcReturnTypes[self->nodeId].value() != self->argument->type.value()) {
        this->report(SemanticErr::TYPE8, self, "the function's return type mismatches the declared type");
    }
}

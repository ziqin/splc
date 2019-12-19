#include "semantic.hpp"
#include "type.hpp"

using namespace AST;
using namespace std;


ScopeSetter::ScopeSetter() {
    BEG_ENTER_HOOK(Program);
        self->scope = make_shared<SymbolTable>(nullptr);
    END_ENTER_HOOK(Program);

    BEG_ENTER_HOOK(FunDef);
        self->scope = parent->scope;
        auto inFuncScope = make_shared<SymbolTable>(self->scope);
        for (ParamDec * para: self->declarator->parameters) {
            para->scope = inFuncScope;
        }
        self->body->scope = inFuncScope;
        // TODO: check whether var definitions in body block are allowed to overwrite function parameters in C language
    END_ENTER_HOOK(FunDef);

    BEG_ENTER_HOOK(ForStmt);
        self->scope = make_shared<SymbolTable>(parent->scope);
    END_ENTER_HOOK(ForStmt);

    BEG_ENTER_HOOK(CompoundStmt);
        self->scope = make_shared<SymbolTable>(parent->scope);
    END_ENTER_HOOK(CompoundStmt);
}

optional<Hook> ScopeSetter::getPreHook(type_index type) {
    auto dedicatedHook = Walker::getPreHook(type);
    return dedicatedHook ? dedicatedHook : [](Node *self, Node *parent) {
        if (self->scope == nullptr) { // scope has not been specified by its parent
            self->scope = parent->scope;
        }
    };
}


void SemanticAnalyzer::report(SemanticErr errType, Node *cause, const std::string& msg) {
    errs.emplace_back(errType, cause, msg);
    nodesWithErr.insert(cause->id);
}

bool SemanticAnalyzer::hasErr(Node *node) const {
    return nodesWithErr.find(node->id) != nodesWithErr.end();
}


StructInitializer::StructInitializer(vector<SemanticErrRecord>& errStore): SemanticAnalyzer(errStore) {
    BEG_ENTER_HOOK(StructDef);
        const string& name = self->specifier->identifier;
        if (this->structures.find(name) != this->structures.end()) {
            this->report(ERR_TYPE15, self, "redefine the same structure type");
        } else {
            this->structures[name] = self->specifier->type;
        }
    END_ENTER_HOOK(StructDef);

    BEG_LEAVE_HOOK(StructDef);
        if (!this->hasErr(self)) {
            Shared<Type> type = this->structures[self->specifier->identifier];
            auto structType = new StructType;
            for (Def *def: self->specifier->definitions) {
                Shared<Type> fieldType = def->specifier->type;
                for (Dec *dec: def->declarations) {
                    structType->fields.push_back(make_pair(fieldType, dec->declarator->identifier));
                }
            }
            type.set(structType);
        }
    END_LEAVE_HOOK(StructDef);

    BEG_ENTER_HOOK(StructSpecifier);
        auto typeItr = this->structures.find(self->identifier);
        if (typeItr == this->structures.end()) {
            this->report(ERR_TYPE0, self, "struct undefined");
        } else {
            self->type = typeItr->second;
        }
    END_ENTER_HOOK(StructSpecifier);
}


// after struct initializer finishes its walk
SymbolSetter::SymbolSetter(vector<SemanticErrRecord>& errStore): SemanticAnalyzer(errStore) {
    BEG_ENTER_HOOK(ExtVarDef);
        for (VarDec *var: self->varDecs) {
            this->typeRefs[var->id] = self->specifier->type;
        }
    END_ENTER_HOOK(ExtVarDef);

    BEG_ENTER_HOOK(ParamDec);
        this->typeRefs[self->declarator->id] = self->specifier->type;
    END_ENTER_HOOK(ParamDec);

    BEG_ENTER_HOOK(FunDef);
        this->typeRefs[self->declarator->id] = self->specifier->type;
    END_ENTER_HOOK(FunDef);

    BEG_LEAVE_HOOK(FunDec);
        if (self->scope->canOverwrite(self->identifier)) {
            FunctionType *type = new FunctionType(this->typeRefs[self->id]);
            for (ParamDec *para: self->parameters) {
                type->parameters.push_back(para->specifier->type);
            }
            self->scope->setType(self->identifier, Shared<Type>(type));
        } else {
            this->report(ERR_TYPE4, parent, "function is redefined");
        }
    END_LEAVE_HOOK(FunDec);

    BEG_ENTER_HOOK(Def);
        for (Dec *dec: self->declarations) {
            this->typeRefs[dec->id] = self->specifier->type;
        }
    END_ENTER_HOOK(Def);

    BEG_ENTER_HOOK(Dec);
        this->typeRefs[self->declarator->id] = this->typeRefs[self->id];
    END_ENTER_HOOK(Dec);

    BEG_LEAVE_HOOK(VarDec);
        if (self->scope->canOverwrite(self->identifier)) {
            self->scope->setType(self->identifier, this->typeRefs[self->id]);
        } else {
            this->report(ERR_TYPE3, self, "variable is redefined in the same scope");
        }
    END_LEAVE_HOOK(VarDec);

    BEG_LEAVE_HOOK(ArrDec);
        Shared<Type> type = this->typeRefs[self->id];
        for (auto dim = self->dimensions.rbegin(); dim != self->dimensions.rend(); ++dim) {
            type = makeType<ArrayType>(type, *dim);
        }
        self->scope->setType(self->identifier, type);
    END_LEAVE_HOOK(VarDec);
}


TypeSynthesizer::TypeSynthesizer(std::vector<SemanticErrRecord>& errStore): SemanticAnalyzer(errStore) {
    BEG_LEAVE_HOOK(IdExp);
        optional<Shared<Type>> defined = self->scope->getType(self->identifier);
        if (defined) {
            self->type = defined.value();
        } else {
            this->report(ERR_TYPE1, self, "variable is used without definition");
        }
    END_LEAVE_HOOK(IdExp);

    BEG_LEAVE_HOOK(CallExp);
        for (auto arg: self->arguments) {
            if (this->hasErr(arg))
                return;
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
                    this->report(ERR_TYPE9, self, "the function's arguments mismatch the declared parameters");
                }
            } catch (const exception& e) {
                this->report(ERR_TYPE11, self, "applying function invocation operator on non-function names");
            }
        } else {
            this->report(ERR_TYPE2, self, "function is invoked without definition");
        }
    END_LEAVE_HOOK(CallExp);

    // TODO: check type compatibility instead of equivalence
    BEG_LEAVE_HOOK(AssignExp);
        if (this->hasErr(self->left) && this->hasErr(self->right)) return;
        if (*self->left->type != *self->right->type)
            this->report(ERR_TYPE5, self, "unmatched types on both sides of assignment operator");
        if (typeid(self->left) != typeid(IdExp*) &&
            typeid(self->left) != typeid(ArrayExp*) &&
            typeid(self->left) != typeid(MemberExp*)
        )   this->report(ERR_TYPE6, self, "rvalue on the left side of assignment operator");
        self->type = self->left->type;
    END_LEAVE_HOOK(AssignExp);

    BEG_LEAVE_HOOK(UnaryExp);
        if (this->hasErr(self->argument)) return;
        try {
            const PrimitiveType& type = as<PrimitiveType>(self->argument->type);
            if (type == TYPE_CHAR) this->report(ERR_TYPE7, self, "unmatched operand");
            else self->type = self->argument->type;
        } catch (const exception& e) {
            this->report(ERR_TYPE7, self, "unmatched operand");
        }
    END_LEAVE_HOOK(UnaryExp);

    BEG_LEAVE_HOOK(BinaryExp);
        if (this->hasErr(self->left) || this->hasErr(self->right)) return;
        Shared<Type> left = self->left->type, right = self->right->type;
        if (typeid(left.get()) != typeid(PrimitiveType*) ||
            typeid(right.get()) != typeid(PrimitiveType*) ||
            *left != *right
        ) {
            this->report(ERR_TYPE7, self, "unmatched operands");
        } else {
            self->type = left;
        }
    END_LEAVE_HOOK(BinaryExp);

    BEG_LEAVE_HOOK(MemberExp);
        if (this->hasErr(self->subject)) return;
        Shared<Type> type = self->subject->type;
        try {
            const StructType& structType = as<StructType>(type);
            Shared<Type> fieldType = structType.getFieldType(self->member);
            if (fieldType == nullptr) {
                this->report(ERR_TYPE14, self, "accessing an undefined structure member");
            } else {
                self->type = fieldType;
            }
        } catch (const exception& e) {
            this->report(ERR_TYPE13, self, "accessing member of non-structure variable");
        }
    END_LEAVE_HOOK(MemberExp);

    BEG_LEAVE_HOOK(ArrayExp);
        if (this->hasErr(self->subject) || this->hasErr(self->index)) return;
        const ArrayType *arrayType = dynamic_cast<const ArrayType*>(self->subject->type.get());
        const PrimitiveType *indexType = dynamic_cast<const PrimitiveType*>(self->index->type.get());
        if (arrayType == nullptr) {
            this->report(ERR_TYPE10, self, "applying indexing operator on non-array type variables");
        }
        if (indexType == nullptr || !(*indexType == TYPE_INT)) {
            this->report(ERR_TYPE12, self, "array indexing with non-integer type expression");
        } else if (arrayType != nullptr) {
            self->type = arrayType->baseType;
        }
    END_LEAVE_HOOK(ArrayExp);

    BEG_LEAVE_HOOK(FunDef);
        Shared<Type> definedReturnType = self->specifier->type;
        auto& statements = self->body->body;
        if (statements.size() > 0) {
            const ReturnStmt *lastStatement = dynamic_cast<const ReturnStmt*>(*statements.rbegin());
            if (lastStatement != nullptr && *(lastStatement->argument->type) != *definedReturnType)
                this->report(ERR_TYPE8, self, "the function's return value type mismatches the declared type");
        }
    END_LEAVE_HOOK(FunDef);
}


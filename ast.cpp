#include <algorithm>
#include <cassert>
#include <string>
#include "ast.hpp"
#include "cst.hpp"

using namespace AST;
using namespace std;

static const char * invalidCstNodeType = "invalid CST node type";


/* Expressions */

using ExpPtr = unique_ptr<Exp>;

ExpPtr Exp::createExp(const CST::Node& node) {
    switch (node.nodeType) {
    case CST::ID:
        return ExpPtr(new IdExp(dynamic_cast<const CST::StrNode&>(node)));
    case CST::CHAR:
    case CST::INT:
    case CST::FLOAT:
        return ExpPtr(new LiteralExp(node));
    case CST::Exp: {
        auto& ntNode = dynamic_cast<const CST::NtNode&>(node);
        switch (ntNode.children.size()) {
        case 1:
            return createExp(*ntNode.children[0]);
        case 2:
            return ExpPtr(new UnaryExp(ntNode));
        case 3:
            switch (ntNode.children[1]->nodeType) {
            case CST::ASSIGN:   return ExpPtr(new AssignExp(ntNode));
            case CST::Exp:      return createExp(*ntNode.children[1]);
            case CST::LP:       return ExpPtr(new CallExp(ntNode));
            case CST::DOT:      return ExpPtr(new MemberExp(ntNode));
            default:            return ExpPtr(new BinaryExp(ntNode));
            }
        case 4:
            if (ntNode.children[1]->nodeType == CST::LP) return ExpPtr(new CallExp(ntNode));
            else return ExpPtr(new ArrayExp(ntNode));
        }
    }
    default:
        throw invalid_argument(invalidCstNodeType);
    }
}

LiteralExp::LiteralExp(const CST::Node& literalNode) {
    switch (literalNode.nodeType) {
    case CST::CHAR:
        type = shared_ptr<Type>(new PrimitiveType(PrimitiveType::TYPE_CHAR));
        charVal = dynamic_cast<const CST::CharNode&>(literalNode).value;
        break;
    case CST::INT:
        type = shared_ptr<Type>(new PrimitiveType(PrimitiveType::TYPE_INT));
        intVal = dynamic_cast<const CST::IntNode&>(literalNode).value;
        break;
    case CST::FLOAT:
        type = shared_ptr<Type>(new PrimitiveType(PrimitiveType::TYPE_FLOAT));
        floatVal = dynamic_cast<const CST::FloatNode&>(literalNode).value;
        break;
    default:
        throw invalid_argument(invalidCstNodeType);
    }
}

IdExp::IdExp(const CST::StrNode& idNode):
    identifier(idNode.value)
{}

ArrayExp::ArrayExp(const CST::NtNode& node):
    subject(createExp(*node.children[0])),
    index(createExp(*node.children[2]))
{}

MemberExp::MemberExp(const CST::NtNode& node):
    subject(createExp(*node.children[0])),
    member(new IdExp(dynamic_cast<const CST::StrNode&>(*node.children[2])))
{}

UnaryExp::UnaryExp(const CST::NtNode& node):
    opt(node.children[0]->nodeType == CST::MINUS ? Operator::OPT_MINUS : Operator::OPT_NOT),
    argument(createExp(*node.children[1]))
{}

BinaryExp::BinaryExp(const CST::NtNode& node):
    left(createExp(*node.children[0])),
    right(createExp(*node.children[2]))
{
    switch (node.children[1]->nodeType) {
    case CST::AND:   opt = Operator::OPT_AND;     break;
    case CST::OR:    opt = Operator::OPT_OR;      break;
    case CST::LT:    opt = Operator::OPT_LT;      break;
    case CST::LE:    opt = Operator::OPT_LE;      break;
    case CST::GT:    opt = Operator::OPT_GT;      break;
    case CST::GE:    opt = Operator::OPT_GE;      break;
    case CST::NE:    opt = Operator::OPT_NE;      break;
    case CST::EQ:    opt = Operator::OPT_EQ;      break;
    case CST::PLUS:  opt = Operator::OPT_PLUS;    break;
    case CST::MINUS: opt = Operator::OPT_MINUS;   break;
    case CST::MUL:   opt = Operator::OPT_MUL;     break;
    case CST::DIV:   opt = Operator::OPT_DIV;     break;
    default: throw invalid_argument(invalidCstNodeType);
    }
}

AssignExp::AssignExp(const CST::NtNode& node):
    left(createExp(*node.children[0])),
    right(createExp(*node.children[2]))
{}

CallExp::CallExp(const CST::NtNode& node):
    callee(new IdExp(dynamic_cast<const CST::StrNode&>(*node.children[0])))
{
    if (node.children.size() == 4) {
        const CST::NtNode * args;
        for (args = dynamic_cast<const CST::NtNode*>(node.children[2]);
             args->nodeType == CST::Args && args->children.size() == 3;
             args = dynamic_cast<const CST::NtNode*>(args->children[2])
        ) {
            arguments.push_back(createExp(*args->children[0]));
        }
        arguments.push_back(createExp(*args->children[0]));
    }
}


/* Statements */

using StmtPtr = unique_ptr<Stmt>;

StmtPtr Stmt::createStmt(const CST::NtNode& node) {
    switch (node.children[0]->nodeType) {
    case CST::Exp:      return StmtPtr(new ExpStmt(node));
    case CST::CompSt:   return StmtPtr(new ComplexStmt(dynamic_cast<const CST::NtNode&>(*node.children[0])));
    case CST::RETURN:   return StmtPtr(new ReturnStmt(node));
    case CST::IF:       return StmtPtr(new IfStmt(node));
    case CST::WHILE:    return StmtPtr(new WhileStmt(node));
    case CST::FOR:      return StmtPtr(new ForStmt(node));
    default:            throw invalid_argument(invalidCstNodeType);
    }
}

ExpStmt::ExpStmt(const CST::NtNode& node):
    expression(Exp::createExp(*node.children[0]))
{}

ReturnStmt::ReturnStmt(const CST::NtNode& node):
    argument(Exp::createExp(*node.children[1]))
{}

IfStmt::IfStmt(const CST::NtNode& node):
    test(Exp::createExp(*node.children[2])),
    consequent(createStmt(dynamic_cast<const CST::NtNode&>(*node.children[4]))),
    alternate(node.children.size() == 7 ? createStmt(dynamic_cast<const CST::NtNode&>(*node.children[6])) : nullptr)
{}

WhileStmt::WhileStmt(const CST::NtNode& node):
    test(Exp::createExp(*node.children[2])),
    body(createStmt(dynamic_cast<const CST::NtNode&>(*node.children[4])))
{}

ForStmt::ForStmt(const CST::NtNode& node) {
    assert(node.nodeType == CST::Stmt && node.children.size() == 9);
    init = node.children[2] ? Exp::createExp(*node.children[2]) : nullptr;
    test = node.children[4] ? Exp::createExp(*node.children[4]) : nullptr;
    update = node.children[6] ? Exp::createExp(*node.children[6]) : nullptr;
    body = createStmt(dynamic_cast<const CST::NtNode&>(*node.children[node.children.size() - 1]));
}

static vector<unique_ptr<VarDef>> createDefs(const CST::NtNode& node);

ComplexStmt::ComplexStmt(const CST::NtNode& node) {
    // DefList
    definitions = createDefs(dynamic_cast<const CST::NtNode&>(*node.children[1]));

    // StmtList
    for (auto list = dynamic_cast<const CST::NtNode*>(node.children[2]);
         list->nodeType == CST::StmtList && list->children.size() == 2;
         list = dynamic_cast<const CST::NtNode*>(list->children[1])
    ) {
        body.emplace_back(createStmt(dynamic_cast<const CST::NtNode&>(*list->children[0])));
    }
}


/* Definitions */

static string processVarDec(shared_ptr<Type>& type, const CST::NtNode& node) {
    if (node.children.size() == 4) {
        int index = dynamic_cast<const CST::IntNode&>(*node.children[2]).value;
        type = shared_ptr<Type>(new ArrayType(type, size_t(index)));
        return processVarDec(type, dynamic_cast<const CST::NtNode&>(*node.children[0]));
    } else {
        assert(node.children[0]->nodeType == CST::ID);
        return dynamic_cast<const CST::StrNode&>(*node.children[0]).value;
    }
}

// VarDef::VarDef(shared_ptr<Type> type, const CST::NtNode& node) {
//     this->type = type;
//     identifier = processVarDec(this->type, dynamic_cast<const CST::NtNode&>(*node.children[0]));
// }

static unique_ptr<VarDef> dec2varDef(shared_ptr<Type> type, const CST::NtNode& dec) {
    assert(dec.nodeType == CST::Dec);
    // unique_ptr<VarDef> varDef(new VarDef(type, dec));
    unique_ptr<VarDef> varDef(new VarDef);
    varDef->type = type;
    varDef->identifier = processVarDec(varDef->type, dynamic_cast<const CST::NtNode&>(*dec.children[0]));
    if (dec.children.size() == 3) {
        varDef->init = Exp::createExp(*dec.children[2]);
    }
    return varDef;
}

static vector<unique_ptr<VarDef>> decList2varDefs(shared_ptr<Type> type, const CST::NtNode& decList) {
    assert(decList.nodeType == CST::DecList);
    vector<unique_ptr<VarDef>> varDefs;
    const CST::NtNode * list;
    for (list = &decList;
         list->nodeType == CST::DecList && list->children.size() == 3;
         list = dynamic_cast<const CST::NtNode*>(list->children[2])
    ) {
        varDefs.push_back(dec2varDef(type, dynamic_cast<const CST::NtNode&>(*list->children[0])));
    }
    varDefs.emplace_back(dec2varDef(type, dynamic_cast<const CST::NtNode&>(*list->children[0])));
    return varDefs;
}

static shared_ptr<Type> specifier2type(const CST::NtNode& node);

// input: Def or DefList
static vector<unique_ptr<VarDef>> createDefs(const CST::NtNode& node) {
    vector<unique_ptr<VarDef>> definitions;
    if (node.nodeType == CST::Def) {
        shared_ptr<Type> type = specifier2type(dynamic_cast<const CST::NtNode&>(*node.children[0]));
        auto varDefs = decList2varDefs(type, dynamic_cast<const CST::NtNode&>(*node.children[1]));
        definitions.reserve(varDefs.size());
        move(varDefs.begin(), varDefs.end(), back_inserter(definitions));
    } else if (node.nodeType == CST::DefList) {
        for (const CST::NtNode * defList = &node;
             defList->children.size() == 2;
             defList = dynamic_cast<const CST::NtNode*>(defList->children[1])
        ) {
            auto defs = createDefs(dynamic_cast<const CST::NtNode&>(*defList->children[0]));
            definitions.reserve(defs.size() + definitions.size());
            move(defs.begin(), defs.end(), back_inserter(definitions));
        }
    }
    return definitions;
}

static shared_ptr<Type> specifier2type(const CST::NtNode& node) {
    assert(node.nodeType == CST::Specifier);
    if (node.children[0]->nodeType == CST::TYPE) {
        auto& typeNode = dynamic_cast<const CST::StrNode&>(*node.children[0]);
        if (typeNode.value == "char") {
            return shared_ptr<Type>(new PrimitiveType(PrimitiveType::TYPE_CHAR));
        } else if (typeNode.value == "int") {
            return shared_ptr<Type>(new PrimitiveType(PrimitiveType::TYPE_INT));
        } else if (typeNode.value == "float") {
            return shared_ptr<Type>(new PrimitiveType(PrimitiveType::TYPE_FLOAT));
        } else {
            throw invalid_argument(invalidCstNodeType);
        }
    } else if (node.children[0]->nodeType == CST::StructSpecifier) {
        const CST::NtNode& child = dynamic_cast<const CST::NtNode&>(*node.children[0]);
        string structName = dynamic_cast<const CST::StrNode&>(*child.children[1]).value;
        if (child.children.size() == 2) {
            return shared_ptr<Type>(new TypeAlias(structName, nullptr));
        } else if (child.children.size() == 5) {
            StructType * structType = new StructType;
            auto defList = createDefs(dynamic_cast<const CST::NtNode&>(*child.children[3]));
            for (auto& def: defList) {
                structType->fields.push_back(make_pair(move(def->type), def->identifier));
            }
            return shared_ptr<Type>(new TypeAlias(structName, shared_ptr<Type>(structType)));
        } else {
            throw invalid_argument(invalidCstNodeType);
        }
    } else {
        throw invalid_argument(invalidCstNodeType);
    }
}

static unique_ptr<VarDef> createParamDec(const CST::NtNode& node) {
    assert(node.nodeType == CST::ParamDec);
    unique_ptr<VarDef> varDef(new VarDef);
    varDef->type = specifier2type(dynamic_cast<const CST::NtNode&>(*node.children[0]));
    varDef->identifier = processVarDec(varDef->type, dynamic_cast<const CST::NtNode&>(*node.children[1]));
    return varDef;
}

StructDef::StructDef(const CST::NtNode& node) {
    assert(node.nodeType == CST::ExtDef && node.children[0]->nodeType == CST::Specifier);
    type = specifier2type(dynamic_cast<const CST::NtNode&>(*node.children[0]));
    identifier = dynamic_cast<const TypeAlias&>(*type).name;
}

FunctionDef::FunctionDef(const CST::NtNode& node) {
    assert(node.nodeType == CST::ExtDef && node.children.size() == 3 && node.children[1]->nodeType == CST::FunDec);
    type = specifier2type(dynamic_cast<const CST::NtNode&>(*node.children[0]));
    auto& declarator = dynamic_cast<const CST::NtNode&>(*node.children[1]);
    identifier = dynamic_cast<const CST::StrNode&>(*declarator.children[0]).value;
    // parameters
    if (declarator.children.size() == 4) {
        const CST::NtNode * varList;
        for(varList = dynamic_cast<const CST::NtNode*>(declarator.children[2]);
            varList->nodeType == CST::VarList && varList->children.size() == 3;
            varList = dynamic_cast<const CST::NtNode*>(varList->children[2])
        ) {
            parameters.emplace_back(createParamDec(dynamic_cast<const CST::NtNode&>(*varList->children[0])));
        }
        parameters.emplace_back(createParamDec(dynamic_cast<const CST::NtNode&>(*varList->children[0])));
    }
    body = unique_ptr<ComplexStmt>(new ComplexStmt(dynamic_cast<const CST::NtNode&>(*node.children[2])));
}

static vector<unique_ptr<VarDef>> createExtDefs(const CST::NtNode& node) {
    assert(node.nodeType == CST::ExtDef && node.children[1]->nodeType == CST::ExtDecList);
    vector<unique_ptr<VarDef>> varDefs;
    shared_ptr<Type> type = specifier2type(dynamic_cast<const CST::NtNode&>(*node.children[0]));
    const CST::NtNode * decList;
    for (decList = dynamic_cast<const CST::NtNode*>(node.children[1]);
         decList->nodeType == CST::ExtDecList && decList->children.size() == 3;
         decList = dynamic_cast<const CST::NtNode*>(decList->children[2])
    ) {
        unique_ptr<VarDef> varDef(new VarDef);
        varDef->type = type;
        varDef->identifier = processVarDec(varDef->type, dynamic_cast<const CST::NtNode&>(*decList->children[0]));
        varDefs.push_back(move(varDef));
    }
    unique_ptr<VarDef> varDef(new VarDef);
    varDef->type = type;
    varDef->identifier = processVarDec(varDef->type, dynamic_cast<const CST::NtNode&>(*decList->children[0]));
    varDefs.push_back(move(varDef));
    return varDefs;
}

Program::Program(const CST::Node& ntNode) {
    assert(ntNode.nodeType == CST::Program);
    const CST::NtNode& node = dynamic_cast<const CST::NtNode&>(ntNode);
    for (auto defList = dynamic_cast<const CST::NtNode*>(node.children[0]);
         defList->nodeType == CST::ExtDefList && defList->children.size() == 2;
         defList = dynamic_cast<const CST::NtNode*>(defList->children[1])
    ) {
        auto& def = dynamic_cast<const CST::NtNode&>(*defList->children[0]);
        switch (def.children[1]->nodeType) {
        case CST::ExtDecList: {
            auto varDefs = createExtDefs(def);
            move(varDefs.begin(), varDefs.end(), back_inserter(definitions));  
            break;
        }
        case CST::SEMI:
            definitions.emplace_back(new StructDef(def));
            break;
        case CST::FunDec:
            definitions.emplace_back(new FunctionDef(def));
            break;
        default:
            throw invalid_argument(invalidCstNodeType);
        }
    }
}

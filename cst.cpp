#include <cstdarg>
#include <cstdio>
#include "cst.hpp"
#include "syntax.tab.h"


static const char * cstTypeToName[] = {
    "INT",      "FLOAT",    "TYPE",     "ID",       "CHAR",     "STRUCT",   "IF",       "ELSE", 
    "WHILE",    "FOR",      "RETURN",   "DOT",      "SEMI",     "COMMA",    "ASSIGN",   "LT",
    "LE",       "GT",       "GE",       "NE",       "EQ",       "PLUS",     "MINUS",    "MUL",
    "DIV",      "AND",      "OR",       "NOT",      "LP",       "RP",       "LB",       "RB",
    "LC",       "RC",
    "Program",      "ExtDefList",       "ExtDef",       "ExtDecList",
    "Specifier",    "StructSpecifier",  "VarDec",       "FunDec",
    "VarList",      "ParamDec",         "CompSt",       "StmtList", 
    "Stmt",         "DefList",          "Def",          "DecList", 
    "Dec",          "Exp",              "Args",
};

inline static void fprintIndent(FILE * fp, int indent) {
    while (indent--) {
        fputc(' ', fp);
    }
}

void CST::Node::fprint(FILE * fp, int indent) {
    fprintIndent(fp, indent);
    std::fprintf(fp, "%s\n", cstTypeToName[nodeType]);
}

CST::CharNode::CharNode(const char * val): CST::Node(CST::CHAR) {
    if (val[0] != '\\') {
        value = val[0];
    } else { // starts with \x
        value = 0;
        for (const char * hex = val + 2; *hex != '\0'; ++hex) {
            value <<= 4;
            if ('0' <= *hex && *hex <= '9') {
                value += *hex - '0';
            } else if ('A' <= *hex && *hex <= 'F') {
                value += *hex - 'A' + 10;
            } else if ('a' <= *hex && *hex <= 'f') {
                value += *hex - 'a' + 10;
            }
        }
    }
}

void CST::CharNode::fprint(FILE * fp, int indent) {
    fprintIndent(fp, indent);
    std::fprintf(fp, "%s: %c\n", cstTypeToName[nodeType], value);
}

CST::IntNode::IntNode(const char * val):
    CST::Node(CST::INT), value(atoi(val))
{}

void CST::IntNode::fprint(FILE * fp, int indent) {
    fprintIndent(fp, indent);
    std::fprintf(fp, "%s: %d\n", cstTypeToName[nodeType], value);
}

CST::FloatNode::FloatNode(const char * val):
    CST::Node(CST::FLOAT), value(atof(val)) // str -> double -> float
{}

void CST::FloatNode::fprint(FILE * fp, int indent) {
    fprintIndent(fp, indent);
    const char * fmt = value >= 1e-4 ? "%s: %f\n" : "%s: %E\n";
    std::fprintf(fp, fmt, cstTypeToName[nodeType], value);
}

CST::StrNode::StrNode(enum CST::NodeType type, const char * val):
    CST::Node(type), value(val)
{}

void CST::StrNode::fprint(FILE * fp, int indent) {
    fprintIndent(fp, indent);
    std::fprintf(fp, "%s: %s\n", cstTypeToName[nodeType], value.c_str());
}

CST::NtNode::NtNode(enum CST::NodeType type, const struct YYLTYPE * loc, const std::vector<CST::Node*> & children):
    CST::Node(type), first_line(loc->first_line), children(children)
{}

CST::NtNode::~NtNode() {
    for (auto child: children) delete child;
}

void CST::NtNode::fprint(FILE * fp, int indent) {
    if (children.size() > 0) {
        fprintIndent(fp, indent);
        std::fprintf(fp, "%s (%d)\n", cstTypeToName[nodeType], first_line);
        for (auto child: children) {
            child->fprint(fp, indent + 2);
        }
    }
}

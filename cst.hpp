#ifndef CST_H
#define CST_H

#include <cstdio>
#include <memory>
#include <string>
#include <vector>


struct YYLTYPE;

namespace CST {

enum NodeType {
    INT,
    FLOAT,
    TYPE,
    ID, 
    CHAR,
    STRUCT, 
    IF, 
    ELSE, 
    WHILE, 
    FOR,
    RETURN, 
    DOT, 
    SEMI, 
    COMMA,
    ASSIGN, 
    LT, 
    LE, 
    GT, 
    GE, 
    NE,
    EQ, 
    PLUS, 
    MINUS, 
    MUL,
    DIV, 
    AND, 
    OR, 
    NOT, 
    LP, 
    RP, 
    LB, 
    RB, 
    LC, 
    RC,
    Program, 
    ExtDefList, 
    ExtDef,
    ExtDecList, 
    Specifier, 
    StructSpecifier, 
    VarDec,
    FunDec,
    VarList,
    ParamDec,
    CompSt,
    StmtList,
    Stmt,
    DefList,
    Def,
    DecList,
    Dec,
    Exp,
    Args,
};

struct Node {
    enum NodeType nodeType;
    Node(enum NodeType type): nodeType(type) {}
    virtual ~Node() {}
    virtual void fprint(FILE * fp, int indent);
};

struct CharNode: public Node {
    unsigned char value;
    CharNode(const char * val);
    void fprint(FILE * fp, int indent);
};

struct IntNode: public Node {
    int value;
    IntNode(const char * val);
    void fprint(FILE * fp, int indent);
};

struct FloatNode: public Node {
    float value;
    FloatNode(const char * val);
    void fprint(FILE * fp, int indent);
};

struct StrNode: public Node {
    std::string value;
    StrNode(enum CST::NodeType type, const char * val);
    void fprint(FILE * fp, int indent);
};

// nonterminal
struct NtNode: public Node {
    int first_line;
    std::vector<Node*> children;
    NtNode(enum CST::NodeType type, const YYLTYPE * loc, const std::vector<Node*>& children);
    ~NtNode();
    void fprint(FILE * fp, int indent);
};

} // end of namespace CST


#endif

#ifndef TAC_HPP
#define TAC_HPP

#include <string>
#include <memory>


namespace gen {


struct TacOperand {
    virtual std::string toString() const = 0;
};

struct VariableOperand final: public TacOperand {
    int id;

    VariableOperand(int id): id(id) {}
    std::string toString() const {
        return 't' + std::to_string(id);
    }
};

template <typename T>
struct ConstantOperand final: public TacOperand {
    T value;

    ConstantOperand(T value): value(value) {}
    std::string toString() const {
        return '#' + std::to_string(value);
    }
};

struct PointerOperand: public TacOperand {
    std::string name;
    std::string toString() const {
        return name;
    }
};


// three address code
struct Tac {
    virtual ~Tac() {}
    virtual std::string toString() const = 0;
};


struct LabelTac final: public Tac {
    int no;

    LabelTac(int number): no(number) {}
    std::string toString() const {
        return "LABEL label" + std::to_string(no) + " :";
    }
};

struct FuncTac final: public Tac {
    std::string name;

    FuncTac(const std::string& name): name(name) {}
    std::string toString() const {
        return "FUNCTION " + name + " :";
    }
};

struct AssignTac final: public Tac {
    std::shared_ptr<TacOperand> left, right;

    AssignTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> right):
        left(left), right(right) {}
    std::string toString() const {
        return left->toString() + " := " + right->toString();
    }
};

struct ArithTac: public Tac {
    std::shared_ptr<TacOperand> left, r1, r2;
    virtual char opChar() const = 0;

    ArithTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> r1, std::shared_ptr<TacOperand> r2):
        left(left), r1(r1), r2(r2) {}
    std::string toString() const {
        return left->toString() + " := " + r1->toString() + " " + opChar() + " " + r2->toString();
    }
};

struct AddTac final: public ArithTac {
    AddTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> r1, std::shared_ptr<TacOperand> r2):
        ArithTac(left, r1, r2) {}
    char opChar() const override {
        return '+';
    }
};

struct SubTac final: public ArithTac {
    SubTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> r1, std::shared_ptr<TacOperand> r2):
        ArithTac(left, r1, r2) {}
    char opChar() const override {
        return '-';
    }
};

struct MulTac final: public ArithTac {
    MulTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> r1, std::shared_ptr<TacOperand> r2):
        ArithTac(left, r1, r2) {}
    char opChar() const override {
        return '*';
    }
};

struct DivTac final: public ArithTac {
    DivTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> r1, std::shared_ptr<TacOperand> r2):
        ArithTac(left, r1, r2) {}
    char opChar() const override {
        return '/';
    }
};

struct AddrTac final: public Tac {
    std::shared_ptr<TacOperand> left, right;
    std::string toString() const {
        return left->toString() + " := &" + right->toString();
    }
};

struct FetchTac final: public Tac {
    std::shared_ptr<TacOperand> left, raddr;
    std::string toString() const {
        return left->toString() + " := *" + raddr->toString();
    }
};

struct DerefTac final: public Tac {
    std::shared_ptr<TacOperand> laddr, right;
    std::string toString() const {
        return '*' + laddr->toString() + " := " + right->toString();
    }
};

struct GotoTac final: public Tac {
    int labelNo;

    GotoTac(int labelNo): labelNo(labelNo) {}
    std::string toString() const {
        return "GOTO label" + std::to_string(labelNo);
    }
};

struct IfGotoTac: public Tac {
    std::shared_ptr<TacOperand> c1, c2;
    int labelNo;
    virtual std::string relopStr() const = 0;

    IfGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        c1(c1), c2(c2), labelNo(labelNo) {}
    std::string toString() const {
        return "IF " + c1->toString() + " " + relopStr() + " " + c2->toString() + " GOTO label" + std::to_string(labelNo);
    }
};

struct IfLtGotoTac final: public IfGotoTac {
    IfLtGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(c1, c2, labelNo) {}
    std::string relopStr() const override {
        return "<";
    }
};

struct IfLeGotoTac final: public IfGotoTac {
    IfLeGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(c1, c2, labelNo) {}
    std::string relopStr() const override {
        return "<=";
    }
};

struct IfGtGotoTac final: public IfGotoTac {
    IfGtGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(c1, c2, labelNo) {}
    std::string relopStr() const override {
        return ">";
    }
};

struct IfGeGotoTac final: public IfGotoTac {
    IfGeGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(c1, c2, labelNo) {}
    std::string relopStr() const override {
        return ">=";
    }
};

struct IfNeGotoTac final: public IfGotoTac {
    IfNeGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(c1, c2, labelNo) {}
    std::string relopStr() const override {
        return "!=";
    }
};

struct IfEqGotoTac final: public IfGotoTac {
    IfEqGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(c1, c2, labelNo) {}
    std::string relopStr() const override {
        return "==";
    }
};

struct ReturnTac final: public Tac {
    std::shared_ptr<TacOperand> var;

    ReturnTac(std::shared_ptr<TacOperand> var): var(var) {}
    std::string toString() const {
        return "RETURN " + var->toString();
    }
};

struct DecSpaceTac final: public Tac {
    std::shared_ptr<TacOperand> var;
    int size;
    std::string toString() const {
        return "DEC " + var->toString() + ' ' + std::to_string(size);
    }
};

struct ParamTac final: public Tac {
    std::shared_ptr<TacOperand> p;

    ParamTac(std::shared_ptr<TacOperand> p): p(p) {}
    std::string toString() const {
        return "PARAM " + p->toString();
    }
};

struct ArgTac final: public Tac {
    std::shared_ptr<TacOperand> var;

    ArgTac(std::shared_ptr<TacOperand> var): var(var) {}
    std::string toString() const {
        return "ARG " + var->toString();
    }
};

struct CallTac final: public Tac {
    std::shared_ptr<TacOperand> ret;
    std::string funcName;

    CallTac(std::shared_ptr<TacOperand> ret, const std::string& funcName):
        ret(ret), funcName(funcName) {}
    std::string toString() const {
        return ret->toString() + " := CALL " + funcName;
    }
};

struct ReadTac final: public Tac {
    std::shared_ptr<TacOperand> p;

    ReadTac(std::shared_ptr<TacOperand> p): p(p) {}
    std::string toString() const {
        return "READ " + p->toString();
    }
};

struct WriteTac final: public Tac {
    std::shared_ptr<TacOperand> p;

    WriteTac(std::shared_ptr<TacOperand> p): p(p) {}
    std::string toString() const {
        return "WRITE " + p->toString();
    }
};


template<typename T, typename... Args, typename std::enable_if<std::is_base_of<TacOperand, T>::value>::type* = nullptr>
inline std::shared_ptr<TacOperand> makeTacOp(Args... args) {
    return std::shared_ptr<TacOperand>(new T(args...));
}


} // namespace gen

#endif // TAC_HPP

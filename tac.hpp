#ifndef TAC_HPP
#define TAC_HPP

#include <ostream>
#include <string>
#include <memory>
#include <utility>


namespace ir {


struct TacOperand {
    virtual std::string toString() const = 0;
};

struct VariableOperand final: public TacOperand {
    int id;

    explicit VariableOperand(int id): id(id) {}
    std::string toString() const override {
        return 't' + std::to_string(id);
    }
};

template <typename T>
struct ConstantOperand final: public TacOperand {
    T value;

    explicit ConstantOperand(T value): value(value) {}
    std::string toString() const override {
        return '#' + std::to_string(value);
    }
};

struct PointerOperand: public TacOperand {
    std::string name;
    std::string toString() const override {
        return name;
    }
};


// three address code
struct Tac {
    virtual ~Tac() = default;
    virtual std::string toString() const = 0;
};


struct LabelTac final: public Tac {
    int no;

    explicit LabelTac(int number): no(number) {}
    std::string toString() const override {
        return "LABEL label" + std::to_string(no) + " :";
    }
};

struct FuncTac final: public Tac {
    std::string name;

    explicit FuncTac(std::string name): name(std::move(name)) {}
    std::string toString() const {
        return "FUNCTION " + name + " :";
    }
};

struct AssignTac final: public Tac {
    std::shared_ptr<TacOperand> left, right;

    AssignTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> right):
        left(std::move(left)), right(std::move(right)) {}
    std::string toString() const override {
        return left->toString() + " := " + right->toString();
    }
};

struct ArithTac: public Tac {
    std::shared_ptr<TacOperand> left, r1, r2;
    virtual char opChar() const = 0;

    ArithTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> r1, std::shared_ptr<TacOperand> r2):
        left(std::move(left)), r1(std::move(r1)), r2(std::move(r2)) {}
    std::string toString() const override {
        return left->toString() + " := " + r1->toString() + " " + opChar() + " " + r2->toString();
    }
};

struct AddTac final: public ArithTac {
    AddTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> r1, std::shared_ptr<TacOperand> r2):
        ArithTac(std::move(left), std::move(r1), std::move(r2)) {}
    char opChar() const override {
        return '+';
    }
};

struct SubTac final: public ArithTac {
    SubTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> r1, std::shared_ptr<TacOperand> r2):
        ArithTac(std::move(left), std::move(r1), std::move(r2)) {}
    char opChar() const override {
        return '-';
    }
};

struct MulTac final: public ArithTac {
    MulTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> r1, std::shared_ptr<TacOperand> r2):
        ArithTac(std::move(left), std::move(r1), std::move(r2)) {}
    char opChar() const override {
        return '*';
    }
};

struct DivTac final: public ArithTac {
    DivTac(std::shared_ptr<TacOperand> left, std::shared_ptr<TacOperand> r1, std::shared_ptr<TacOperand> r2):
        ArithTac(std::move(left), std::move(r1), std::move(r2)) {}
    char opChar() const override {
        return '/';
    }
};

struct AddrTac final: public Tac {
    std::shared_ptr<TacOperand> left, right;
    std::string toString() const override {
        return left->toString() + " := &" + right->toString();
    }
};

struct FetchTac final: public Tac {
    std::shared_ptr<TacOperand> left, raddr;
    std::string toString() const override {
        return left->toString() + " := *" + raddr->toString();
    }
};

struct DerefTac final: public Tac {
    std::shared_ptr<TacOperand> laddr, right;
    std::string toString() const override {
        return '*' + laddr->toString() + " := " + right->toString();
    }
};

struct GotoTac final: public Tac {
    int labelNo;

    explicit GotoTac(int labelNo): labelNo(labelNo) {}
    std::string toString() const override {
        return "GOTO label" + std::to_string(labelNo);
    }
};

struct IfGotoTac: public Tac {
    std::shared_ptr<TacOperand> c1, c2;
    int labelNo;
    virtual std::string relopStr() const = 0;

    IfGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        c1(std::move(c1)), c2(std::move(c2)), labelNo(labelNo) {}
    std::string toString() const override {
        return "IF " + c1->toString() + " " + relopStr() + " " + c2->toString() + " GOTO label" + std::to_string(labelNo);
    }
};

struct IfLtGotoTac final: public IfGotoTac {
    IfLtGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(std::move(c1), std::move(c2), labelNo) {}
    std::string relopStr() const override {
        return "<";
    }
};

struct IfLeGotoTac final: public IfGotoTac {
    IfLeGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(std::move(c1), std::move(c2), labelNo) {}
    std::string relopStr() const override {
        return "<=";
    }
};

struct IfGtGotoTac final: public IfGotoTac {
    IfGtGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(std::move(c1), std::move(c2), labelNo) {}
    std::string relopStr() const override {
        return ">";
    }
};

struct IfGeGotoTac final: public IfGotoTac {
    IfGeGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(std::move(c1), std::move(c2), labelNo) {}
    std::string relopStr() const override {
        return ">=";
    }
};

struct IfNeGotoTac final: public IfGotoTac {
    IfNeGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(std::move(c1), std::move(c2), labelNo) {}
    std::string relopStr() const override {
        return "!=";
    }
};

struct IfEqGotoTac final: public IfGotoTac {
    IfEqGotoTac(std::shared_ptr<TacOperand> c1, std::shared_ptr<TacOperand> c2, int labelNo):
        IfGotoTac(std::move(c1), std::move(c2), labelNo) {}
    std::string relopStr() const override {
        return "==";
    }
};

struct ReturnTac final: public Tac {
    std::shared_ptr<TacOperand> var;

    explicit ReturnTac(std::shared_ptr<TacOperand> var): var(std::move(var)) {}
    std::string toString() const override {
        return "RETURN " + var->toString();
    }
};

struct DecSpaceTac final: public Tac {
    std::shared_ptr<TacOperand> var;
    int size;
    std::string toString() const override {
        return "DEC " + var->toString() + ' ' + std::to_string(size);
    }
};

struct ParamTac final: public Tac {
    std::shared_ptr<TacOperand> p;

    explicit ParamTac(std::shared_ptr<TacOperand> p): p(std::move(p)) {}
    std::string toString() const override {
        return "PARAM " + p->toString();
    }
};

struct ArgTac final: public Tac {
    std::shared_ptr<TacOperand> var;

    explicit ArgTac(std::shared_ptr<TacOperand> var): var(std::move(var)) {}
    std::string toString() const override {
        return "ARG " + var->toString();
    }
};

struct CallTac final: public Tac {
    std::shared_ptr<TacOperand> ret;
    std::string funcName;

    CallTac(std::shared_ptr<TacOperand> ret, std::string  funcName):
        ret(std::move(ret)), funcName(std::move(funcName)) {}
    std::string toString() const override {
        return ret->toString() + " := CALL " + funcName;
    }
};

struct ReadTac final: public Tac {
    std::shared_ptr<TacOperand> p;

    explicit ReadTac(std::shared_ptr<TacOperand> p): p(std::move(p)) {}
    std::string toString() const override {
        return "READ " + p->toString();
    }
};

struct WriteTac final: public Tac {
    std::shared_ptr<TacOperand> p;

    explicit WriteTac(std::shared_ptr<TacOperand> p): p(std::move(p)) {}
    std::string toString() const override {
        return "WRITE " + p->toString();
    }
};


template<typename T, typename... Args, typename std::enable_if<std::is_base_of<TacOperand, T>::value>::type* = nullptr>
inline std::shared_ptr<TacOperand> makeTacOp(Args... args) {
    return std::shared_ptr<TacOperand>(new T(args...));
}

} // namespace ir


inline std::ostream& operator<<(std::ostream& out, const ir::Tac& tac) {
    return out << tac.toString();
}

#endif // TAC_HPP

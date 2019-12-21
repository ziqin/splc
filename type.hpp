#ifndef TYPE_H
#define TYPE_H

#include <memory>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>
#include "utils.hpp"

namespace smt {

enum Primitive {
    TYPE_CHAR,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_AUTO
};

struct Type {
    virtual ~Type() {}
    virtual bool operator==(const Type& other) const = 0;
    bool operator!=(const Type& other) const {
        return !(*this == other);
    }
};

struct PrimitiveType: public Type {
    Primitive primitive;

    PrimitiveType(Primitive primitive): primitive(primitive) {}
    bool operator==(const PrimitiveType& other) const;
    bool operator==(const Type& other) const override;
    bool operator==(Primitive p) const;
};

struct ArrayType: public Type {
    Shared<Type> baseType;
    size_t size;

    ArrayType(Shared<Type> baseType, size_t size):
        baseType(baseType), size(size) {}
    bool operator==(const ArrayType& other) const;
    bool operator==(const Type& other) const;
};

using StructField = std::pair<Shared<Type>, std::string>;

struct StructType: public Type {
    std::vector<StructField> fields;

    StructType(const std::vector<StructField>& fields = {}): fields(fields) {}
    bool operator==(const StructType& other) const;
    bool operator==(const Type& other) const;
    Shared<Type> getFieldType(const std::string& name) const;
};

struct FunctionType: public Type {
    Shared<Type> returned;
    std::vector<Shared<Type>> parameters;

    FunctionType(Shared<Type> returned, const std::vector<Shared<Type>>& parameters = {}):
        returned(returned), parameters(parameters) {}
    bool operator==(const FunctionType& other) const;
    bool operator==(const Type& other) const;
};

struct TypeAlias: public Type {
    std::string name;
    Shared<Type> base;

    TypeAlias(std::string name, Shared<Type> base):
        name(name), base(base) {}

    bool operator==(const TypeAlias&) const;
    bool operator==(const Type&) const;
};

inline static bool operator==(const Type& a, const TypeAlias& b) {
    return b == a;
}


template<typename T, typename... Args, typename std::enable_if<std::is_base_of<Type, T>::value>::type* = nullptr>
inline Shared<Type> makeType(Args... args) {
    return Shared<Type>(new T(args...));
}


template<typename T, typename std::enable_if<std::is_base_of<Type, T>::value>::type* = nullptr>
inline T& as(Shared<Type> type) {
    return dynamic_cast<T&>(type.value());
}


} // end of namespace AST

#endif

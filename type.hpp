#ifndef TYPE_H
#define TYPE_H

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>
#include "utils.hpp"

namespace smt {

enum class Primitive {
    CHAR,
    INT,
    FLOAT,
    AUTO
};

struct Type {
    virtual ~Type() = default;
    virtual bool operator==(const Type& other) const = 0;
    bool operator!=(const Type& other) const {
        return !(*this == other);
    }
};

struct PrimitiveType final: public Type {
    Primitive primitive;

    explicit PrimitiveType(Primitive primitive): primitive(primitive) {}
    bool operator==(const PrimitiveType& other) const;
    bool operator==(const Type& other) const override;
    bool operator==(Primitive p) const;
};

struct ArrayType final: public Type {
    Shared<Type> baseType;
    size_t size;

    ArrayType(Shared<Type> baseType, size_t size):
        baseType(std::move(baseType)), size(size) {}
    bool operator==(const ArrayType& other) const;
    bool operator==(const Type& other) const override;
};

using StructField = std::pair<Shared<Type>, std::string>;

struct StructType final: public Type {
    std::vector<StructField> fields;

    explicit StructType(std::vector<StructField> fields = {}): fields(std::move(fields)) {}
    bool operator==(const StructType& other) const;
    bool operator==(const Type& other) const override;
    Shared<Type> getFieldType(const std::string& name) const;
};

struct FunctionType final: public Type {
    Shared<Type> returned;
    std::vector<Shared<Type>> parameters;

    explicit FunctionType(Shared<Type> returned, std::vector<Shared<Type>> parameters = {}):
        returned(std::move(returned)), parameters(std::move(parameters)) {}
    bool operator==(const FunctionType& other) const;
    bool operator==(const Type& other) const override;
};

struct TypeAlias: public Type {
    std::string name;
    Shared<Type> base;

    TypeAlias(std::string name, Shared<Type> base):
        name(std::move(name)), base(std::move(base)) {}

    bool operator==(const TypeAlias&) const;
    bool operator==(const Type&) const override;
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
    return dynamic_cast<T&>(*type);
}


} // end of namespace ast

#endif

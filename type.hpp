#ifndef TYPE_H
#define TYPE_H

#include <memory>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>


namespace AST {

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
    bool operator==(const Type& other) const;
};

struct ArrayType: public Type {
    std::shared_ptr<Type> baseType;
    size_t size;

    ArrayType(std::shared_ptr<Type> baseType, size_t size):
        baseType(baseType), size(size) {}
    bool operator==(const Type& other) const;
};

struct StructType: public Type {
    std::vector<std::pair<std::shared_ptr<Type>, std::string>> fields;

    StructType() {}
    StructType(const std::vector<std::pair<std::shared_ptr<Type>, std::string>>& fields): fields(fields) {}
    bool operator==(const Type& other) const;
    std::shared_ptr<Type> getFieldType(const std::string& name);
};

struct FunctionType: public Type {
    std::shared_ptr<Type> returned;
    std::vector<std::shared_ptr<Type>> parameters;

    FunctionType(std::shared_ptr<Type> returned):
        returned(returned)
    {}

    FunctionType(std::shared_ptr<Type> returned, std::vector<std::shared_ptr<Type>> parameters):
        returned(returned), parameters(parameters)
    {}

    bool operator==(const Type& other) const;
};

struct TypeAlias: public Type {
    std::string name;
    std::shared_ptr<Type> base;

    TypeAlias(std::string name, std::shared_ptr<Type> base):
        name(name), base(base)
    {}

    bool operator==(const TypeAlias&) const;
    bool operator==(const Type&) const;
};

inline static bool operator==(const Type& a, const TypeAlias& b) {
    return b == a;
}


template<typename T, typename... Args, typename std::enable_if<std::is_base_of<Type, T>::value>::type* = nullptr>
std::shared_ptr<Type> make_type(Args... args) {
    return std::shared_ptr<Type>(new T(args...));
}

} // end of namespace AST

#endif

#ifndef TYPE_H
#define TYPE_H

#include <memory>
#include <vector>
#include <string>
#include <typeinfo>
#include <utility>


namespace AST {

struct Type {
    virtual ~Type() {}
    virtual bool operator==(const Type& other) const = 0;
    virtual bool operator!=(const Type& other) const {
        return !(*this == other);
    }
};

struct PrimitiveType: public Type {
    enum Primitive {
        TYPE_CHAR,
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_AUTO
    } primitive;

    PrimitiveType(enum Primitive primitive): primitive(primitive) {}
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

    bool operator==(const Type& other) const;
};

} // end of namespace AST

#endif
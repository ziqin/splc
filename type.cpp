#include <typeinfo>
#include "type.hpp"

using namespace AST;

bool PrimitiveType::operator==(const PrimitiveType& other) const {
    return primitive == other.primitive;
}

bool PrimitiveType::operator==(const Type& other) const {
    if (typeid(TypeAlias) == typeid(other)) return other == *this;
    return typeid(PrimitiveType) == typeid(other) &&
        *this == dynamic_cast<const PrimitiveType&>(other);
}

bool PrimitiveType::operator==(Primitive p) const {
    return primitive == p;
}

bool ArrayType::operator==(const ArrayType& other) const {
    return size == other.size && *baseType == *other.baseType;
}

bool ArrayType::operator==(const Type& other) const {
    if (typeid(TypeAlias) == typeid(other)) return other == *this;
    return typeid(ArrayType) == typeid(other) &&
        *this == dynamic_cast<const ArrayType&>(other);
}

bool StructType::operator==(const StructType& other) const {
    if (fields.size() != other.fields.size()) return false;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (*(fields[i].first) != *(other.fields[i].first))
            return false;
    }
    return true;
}

bool StructType::operator==(const Type& other) const {
    if (typeid(TypeAlias) == typeid(other)) return other == *this;
    return typeid(StructType) == typeid(other) &&
        *this == dynamic_cast<const StructType&>(other);
}

Shared<Type> StructType::getFieldType(const std::string& name) {
    for (auto& field: fields) {
        if (field.second == name)
            return field.first;
    }
    return nullptr;
}

bool FunctionType::operator==(const FunctionType& other) const {
    if ((returned != nullptr && other.returned != nullptr && *returned != *(other.returned)) ||
        parameters.size() != other.parameters.size()) return false;
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (parameters[i] == other.parameters[i]) continue; // don't forget nullptr
        if (*(parameters[i]) != *(other.parameters[i]))
            return false;
    }
    return true;
}

bool FunctionType::operator==(const Type& other) const {
    if (typeid(TypeAlias) == typeid(other)) return other == *this;
    return typeid(FunctionType) == typeid(other) &&
        *this == dynamic_cast<const FunctionType&>(other);
}

bool TypeAlias::operator==(const TypeAlias& other) const {
    return *base == *other.base;
}

bool TypeAlias::operator==(const Type& other) const {
    return *base == other;
}

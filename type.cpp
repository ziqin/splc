#include "type.hpp"

using namespace AST;


bool PrimitiveType::operator==(const Type& other) const {
    try {
        const PrimitiveType& o = dynamic_cast<const PrimitiveType&>(other);
        return primitive == o.primitive;
    } catch (const std::bad_cast& e) {
        return false;
    }
}

bool ArrayType::operator==(const Type& other) const {
    try {
        const ArrayType& o = dynamic_cast<const ArrayType&>(other);
        return baseType == o.baseType && size == o.size;
    } catch (const std::bad_cast& e) {
        return false;
    }
}

bool StructType::operator==(const Type& other) const {
    try {
        const StructType& o = dynamic_cast<const StructType&>(other);
        if (fields.size() != o.fields.size()) return false;
        for (size_t i = 0; i < fields.size(); ++i) {
            if (*(fields[i].first) != *(o.fields[i].first))
                return false;
        }
        return true;
    } catch (const std::bad_cast& e) {
        return false;
    }
}

std::shared_ptr<Type> StructType::getFieldType(const std::string& name) {
    for (auto& field: fields) {
        if (field.second == name)
            return field.first;
    }
    return nullptr;
}

bool FunctionType::operator==(const Type& other) const {
    try {
        const FunctionType& o = dynamic_cast<const FunctionType&>(other);
        if (*returned != *(o.returned) || parameters.size() != o.parameters.size()) return false;
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (*(parameters[i]) != *(o.parameters[i]))
                return false;
        }
        return true;
    } catch (const std::bad_cast& e) {
        return false;
    }
}

bool TypeAlias::operator==(const Type& other) const {
    try {
        const TypeAlias& o = dynamic_cast<const TypeAlias&>(other);
        return *base == *o.base;
    } catch (const std::bad_cast& e) {
        return *base == other;
    }
}

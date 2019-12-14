#include <map>
#include <memory>
#include <string>
#include "type.hpp"

namespace AST {

class SymbolTable {
private:
    std::map<std::string, std::shared_ptr<Type>> table;
    std::shared_ptr<SymbolTable> parent;

public:
    SymbolTable(std::shared_ptr<SymbolTable> parent): parent(parent) {}

    void add(std::string identifier, std::shared_ptr<Type> type) {
        table.insert({ identifier, type });
    }

    std::shared_ptr<Type> getType(std::string identifier) {
        auto found = table.find(identifier);
        if (found != table.end()) return found->second;
        if (parent != nullptr) return parent->getType(identifier);
        return nullptr;
    }

    size_t size() const {
        return table.size() + (parent == nullptr ? 0 : parent->size());
    }

    bool isLowerThan(std::shared_ptr<SymbolTable> scope) const {
        if (scope.get() == this) return true;
        if (parent != nullptr) return parent->isLowerThan(scope);
        return false;
    }
};

} // end of namespace AST

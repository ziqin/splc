#include <map>
#include <optional>
#include <string>
#include "type.hpp"
#include "utils.hpp"

namespace AST {

class SymbolTable {
private:
    std::map<std::string, Shared<Type>> table;
    std::shared_ptr<SymbolTable> parent;

public:
    SymbolTable(std::shared_ptr<SymbolTable> parent): parent(parent) {}

    void setType(const std::string& identifier, Shared<Type> type) {
        table[identifier] = type;
    }

    std::optional<Shared<Type>> getType(const std::string& identifier) {
        auto found = table.find(identifier);
        if (found != table.end()) return found->second;
        if (parent != nullptr) return parent->getType(identifier);
        return std::nullopt;
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

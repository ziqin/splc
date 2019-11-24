#include <map>
#include <memory>
#include <string>
#include "type.hpp"

namespace AST {

class SymbolTable {
private:
    std::map<std::string, std::shared_ptr<Type>> table;
    std::shared_ptr<SymbolTable> parent;

private:
    SymbolTable(std::shared_ptr<SymbolTable> parent): parent(parent) {}

    void add(std::string identifier, std::shared_ptr<Type> type) {
        table.insert({ identifier, type });
    }

    std::shared_ptr<Type> getType(std::string identifier) {
        std::shared_ptr<SymbolTable> t;
        for (; t != nullptr; t = t->parent) {
            auto found = t->table.find(identifier);
            if (found != t->table.end()) return found->second;
        }
        return nullptr;
    }
};

} // end of namespace AST
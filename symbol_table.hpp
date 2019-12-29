#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <map>
#include <optional>
#include <string>
#include <typeinfo>
#include "type.hpp"
#include "utils.hpp"

namespace smt {


struct Symbol {
    Shared<Type> type;
    int symbolId;
};


class SymbolTable {
private:
    std::map<std::string, Symbol> table;
    std::shared_ptr<SymbolTable> parent;

    inline static int globalSymbolSeq = 0;
    inline static int globalLabelSeq = 0;

    Symbol& getSymbol(const std::string& identifier) {
        auto found = table.find(identifier);
        if (found != table.end()) return found->second;
        if (parent != nullptr) return parent->getSymbol(identifier);
        throw std::invalid_argument("identifier not found");
    }

public:
    SymbolTable() {}
    SymbolTable(std::shared_ptr<SymbolTable> parent):
        parent(parent) {}

    void setType(const std::string& identifier, Shared<Type> type) {
        table[identifier] = Symbol { type, 0 };
    }

    std::optional<Shared<Type>> getType(const std::string& identifier) {
        auto found = table.find(identifier);
        if (found != table.end()) return found->second.type;
        if (parent != nullptr) return parent->getType(identifier);
        return std::nullopt;
    }

    int getId(const std::string& identifier) {
        auto& symbol = getSymbol(identifier);
        return symbol.symbolId > 0 ? symbol.symbolId : (symbol.symbolId = ++globalSymbolSeq);
    }

    int createPlace() {
        return ++globalSymbolSeq;
    }

    int createLabel() {
        return ++globalLabelSeq;
    }

    size_t size() const {
        return table.size() + (parent == nullptr ? 0 : parent->size());
    }

    bool isLowerThan(std::shared_ptr<SymbolTable> scope) const {
        if (scope.get() == this) return true;
        if (parent != nullptr) return parent->isLowerThan(scope);
        return false;
    }

    bool canOverwrite(const std::string& name) const {
        return table.find(name) == table.end();
    }
};

} // end of namespace AST


#endif // SYMBOL_TABLE_HPP

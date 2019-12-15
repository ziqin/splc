#include <iostream>
#include "semantic.hpp"

using namespace AST;
using namespace std;


ostream& operator<<(std::ostream& out, const Location& loc) {
    return out << "[start: line " << loc.start.line <<", col " << loc.start.column
        << "; end: line " << loc.end.line << ", col " << loc.end.column << ']';
}

std::optional<Hook> DumpWalker::getPreHook(std::type_index type) {
    return [this](const Node * self, const Node * parent) {
        int parentIndent = parent ? indent[(void*)parent] : 0;
        for (int i = 0; i < parentIndent; ++i) cout << ' ';
        cout << typeid(*self).name() << ' ' << self->loc << endl;
        this->indent[(void *)self] = parentIndent + 2;
    };
}

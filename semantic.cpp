#include <iostream>
#include "semantic.hpp"

using namespace AST;
using namespace std;


ostream& operator<<(std::ostream& out, const Location& loc) {
    return out << "[loc: " << loc.start.line << '$' << loc.start.column
        << " ~ " << loc.end.line << '$' << loc.end.column << ']';
}

std::optional<Hook> DumpWalker::getPreHook(std::type_index type) {
    return [this](const Node * self, const Node * parent) {
        int parentIndent = parent ? indent[parent->id] : 0;
        for (int i = 0; i < parentIndent; ++i) cout << ' ';
        cout << typeid(*self).name() << ' ' << self->loc << endl;
        this->indent[self->id] = parentIndent + 2;
    };
}

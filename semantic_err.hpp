#ifndef SEMANTIC_ERR_HPP
#define SEMANTIC_ERR_HPP

#include <ostream>
#include <string>
#include <utility>

namespace ast {
struct Node;
}

namespace smt {

enum class SemanticErr {
    TYPE0,
    TYPE1,
    TYPE2,
    TYPE3,
    TYPE4,
    TYPE5,
    TYPE6,
    TYPE7,
    TYPE8,
    TYPE9,
    TYPE10,
    TYPE11,
    TYPE12,
    TYPE13,
    TYPE14,
    TYPE15
};

struct SemanticErrRecord {
    SemanticErr err;
    ast::Node *cause;
    std::string msg;

    SemanticErrRecord(SemanticErr err, ast::Node *cause, std::string msg):
        err(err), cause(cause), msg(std::move(msg)) {}
};

} // namespace smt


inline std::ostream& operator<<(std::ostream& out, const smt::SemanticErrRecord& err) {
    return out << "Error type " << int(err.err) - int(smt::SemanticErr::TYPE0)
               << " at Line " << err.cause->loc.end.line << ": " << err.msg;
}


#endif // SEMANTIC_ERR_HPP

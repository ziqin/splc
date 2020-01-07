#ifndef SEMANTIC_ERR_HPP
#define SEMANTIC_ERR_HPP

#include <ostream>
#include <string>

namespace ast {
struct Node;
}

namespace smt {

enum SemanticErr {
    ERR_TYPE0,
    ERR_TYPE1,
    ERR_TYPE2,
    ERR_TYPE3,
    ERR_TYPE4,
    ERR_TYPE5,
    ERR_TYPE6,
    ERR_TYPE7,
    ERR_TYPE8,
    ERR_TYPE9,
    ERR_TYPE10,
    ERR_TYPE11,
    ERR_TYPE12,
    ERR_TYPE13,
    ERR_TYPE14,
    ERR_TYPE15
};

struct SemanticErrRecord {
    SemanticErr err;
    ast::Node *cause;
    std::string msg;

    SemanticErrRecord(SemanticErr err, ast::Node *cause, std::string msg):
        err(err), cause(cause), msg(msg) {}
};

} // namespace smt


inline std::ostream& operator<<(std::ostream& out, const smt::SemanticErrRecord& err) {
    return out << "Error type " << err.err - smt::ERR_TYPE0
               << " at Line " << err.cause->loc.end.line << ": " << err.msg;
}


#endif // SEMANTIC_ERR_HPP

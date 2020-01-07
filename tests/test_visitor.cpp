#include <catch2/catch.hpp>
#include <memory>
#include "ast.hpp"
#include "ast_visitor.hpp"
#include "semantic.hpp"

using namespace std;
using namespace ast;


class FuncDefVisitor: public Visitor {
public:
    void enter(FunDef *self, Node *parent) {
        CHECK(self->declarator->identifier == "foo");
    }

    void leave(CompoundStmt *self, Node *parent) {
        CHECK(self->definitions.size() == 0);
    }
};

TEST_CASE("simple walker works", "[ast-visitor]") {

    SECTION("visiting function definition") {
        auto compStmt = new CompoundStmt({}, {});
        auto specifier = new PrimitiveSpecifier("int");
        auto param = new ParamDec(new PrimitiveSpecifier("char"), new VarDec("a"));
        auto funcDec = new FunDec("foo", { param });
        auto func = make_unique<FunDef>(specifier, funcDec, compStmt);

        auto testWalker = make_unique<FuncDefVisitor>();
        func->traverse({ testWalker.get() }, nullptr);
    }
}

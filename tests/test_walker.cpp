#include <catch2/catch.hpp>
#include <memory>
#include "ast.hpp"
#include "ast_walker.hpp"
#include "semantic.hpp"

using namespace std;
using namespace AST;


class FuncDefWalker: public Walker {
public:
    FuncDefWalker() {
        preHooks[typeid(FunDef)] = [](Node * current, Node * parent) {
            auto self = dynamic_cast<FunDef*>(current);
            CHECK(self->declarator->identifier == "foo");
        };
        postHooks[typeid(CompoundStmt)] = [](Node * current, Node * parent) {
            auto self = dynamic_cast<CompoundStmt*>(current);
            CHECK(self->definitions.size() == 0);
        };
    }
};

TEST_CASE("simple walker works", "[ast-visitor]") {

    SECTION("visiting function definition") {
        auto compStmt = new CompoundStmt({}, {});
        auto specifier = new PrimitiveSpecifier("int");
        auto param = new ParamDec(new PrimitiveSpecifier("char"), new VarDec("a"));
        auto funcDec = new FunDec("foo", { param });
        auto func = make_unique<FunDef>(specifier, funcDec, compStmt);

        auto testWalker = make_unique<FuncDefWalker>();
        func->traverse({ testWalker.get() }, nullptr);
    }
}

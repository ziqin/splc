#include <memory>
#include "ast.hpp"
#include "catch.hpp"
#include "parser.hpp"

using namespace std;
using namespace ast;


TEST_CASE("ast nodes for expressions can be constructed", "[ast-exp]") {

    SECTION("constructing id expressions") {
        CHECK_NOTHROW(IdExp("abc_123_efg"));
        CHECK_NOTHROW(IdExp("_a1"));
        CHECK_THROWS_AS(IdExp("12a"), invalid_argument);
        CHECK_THROWS_AS(IdExp("a("), invalid_argument);
    }

    SECTION("constructing array expressions") {
        auto arrExp = make_unique<ArrayExp>(new IdExp("name"), new LiteralExp(123456));
        CHECK(dynamic_cast<const IdExp*>(arrExp->subject)->identifier == "name");
        CHECK(dynamic_cast<const LiteralExp*>(arrExp->index)->intVal == 123456);
    }

    SECTION("constructing a member expression normally") {
        auto memberExp = make_unique<MemberExp>(new IdExp("name1"), "name2");

        CHECK(dynamic_cast<const IdExp*>(memberExp->subject)->identifier == "name1");
        CHECK(memberExp->member == "name2");
    }

    SECTION("constructing unary expressions") {
        CHECK_NOTHROW(UnaryExp(Operator::PLUS, new LiteralExp(1)));
        CHECK_NOTHROW(UnaryExp(Operator::MINUS, new LiteralExp(2)));
        CHECK_NOTHROW(UnaryExp(Operator::NOT, new IdExp("a")));
        CHECK_THROWS_AS(UnaryExp(Operator::AND, new IdExp("b")), invalid_argument);
    }

    SECTION("constructing binary expressions") {
        CHECK_NOTHROW(BinaryExp(new IdExp("a"), Operator::MUL, new IdExp("b")));
        CHECK_THROWS_AS(BinaryExp(new IdExp("c"), Operator::NOT, new IdExp("d")), invalid_argument);
    }

    SECTION("constructing assignment expression") {
        auto assignExp = make_unique<AssignExp>(new IdExp("lhs"), new LiteralExp('c'));

        CHECK(dynamic_cast<const IdExp*>(assignExp->left)->identifier == "lhs");
        CHECK(dynamic_cast<const LiteralExp*>(assignExp->right)->charVal == 'c');
    }

    SECTION("constructing function call expression") {
        CHECK_NOTHROW(CallExp("f", { new LiteralExp(1), new LiteralExp(3.14) }));
        CHECK_NOTHROW(CallExp("f", {}));
        CHECK_THROWS_AS(CallExp("f", { new LiteralExp(0), nullptr }), invalid_argument);
    }
}

Exp * createExp() {
    return new IdExp("a"); 
}

Stmt * createStmt() {
    return new ExpStmt(new AssignExp(new IdExp("b"), new LiteralExp(0)));
}

TEST_CASE("ast nodes for statements can be constructed and destroyed", "[ast-stmt]") {

    SECTION("constructing/destroying expression statements") {
        REQUIRE_NOTHROW(ExpStmt(createExp()));
        CHECK_THROWS_AS(ExpStmt(nullptr), invalid_argument);
    }

    SECTION("constructing/destroying return statements") {
        CHECK_NOTHROW(ReturnStmt());
        CHECK_NOTHROW(ReturnStmt(createExp()));
    }

    SECTION("constructing/destroying if statements") {
        CHECK_NOTHROW(IfStmt(createExp(), createStmt()));
        CHECK_NOTHROW(IfStmt(createExp(), createStmt(), createStmt()));
        CHECK_THROWS_AS(IfStmt(nullptr, createStmt()), invalid_argument);
        CHECK_THROWS_AS(IfStmt(createExp(), nullptr), invalid_argument);
    }

    SECTION("constructing/destroying while statements") {
        CHECK_NOTHROW(WhileStmt(createExp(), createStmt()));
        CHECK_THROWS_AS(WhileStmt(nullptr, createStmt()), invalid_argument);
        CHECK_THROWS_AS(WhileStmt(createExp(), nullptr), invalid_argument);
        CHECK_THROWS_AS(WhileStmt(nullptr, nullptr), invalid_argument);
    }

    SECTION("constructing/destroying for statements") {
        CHECK_NOTHROW(ForStmt(createExp(), createExp(), createExp(), createStmt()));
        CHECK_NOTHROW(ForStmt(nullptr, nullptr, nullptr, createStmt()));
        CHECK_THROWS_AS(ForStmt(createExp(), createExp(), createExp(), nullptr), invalid_argument);
    }

}


TEST_CASE("ast can be constructed", "[ast-program]") {
    SECTION("constructing an ast from a string") {
        const char * src =
            "int twice(int c) {"
            "  return c + c;"
            "}";
        unique_ptr<ast::Program> ast(parseStr(src));
        REQUIRE(ast != nullptr);
        THEN("there is a function definition") {
            CHECK(ast->extDefs.size() == 1);
            const auto * func = dynamic_cast<const FunDef*>(ast->extDefs[0]);
            CHECK(func != nullptr);
        }
    }
}

#include <catch2/catch.hpp>
#include <memory>
#include "ast_exp.hpp"

using namespace std;
using namespace AST;


TEST_CASE("ast expression nodes can be constructed", "[AST::Exp]") {

    SECTION("constructing array expressions") {
        auto arrExp = make_unique<ArrayExp>(new IdExp("name"), new LiteralExp(123456));

        CHECK(dynamic_cast<const IdExp*>(arrExp->subject)->identifier == "name");
        CHECK(dynamic_cast<const LiteralExp*>(arrExp->index)->intVal == 123456);
    }

    SECTION("constructing a member expression normally") {
        auto memberExp = make_unique<MemberExp>(new IdExp("name1"), new IdExp("name2"));

        CHECK(dynamic_cast<const IdExp*>(memberExp->subject)->identifier == "name1");
        CHECK(dynamic_cast<const IdExp*>(memberExp->member)->identifier == "name2");
    }

    SECTION("construcing a member expression with non-id member") {
        auto name = new IdExp("name");
        auto charExp = new LiteralExp('c');

        CHECK_THROWS_AS(MemberExp(name, charExp), invalid_argument);
    }

    SECTION("constructing unary expressions") {
        CHECK_NOTHROW(UnaryExp(Operator::OPT_PLUS, new LiteralExp(1)));
        CHECK_NOTHROW(UnaryExp(Operator::OPT_MINUS, new LiteralExp(2)));
        CHECK_NOTHROW(UnaryExp(Operator::OPT_NOT, new IdExp("a")));
        CHECK_THROWS_AS(UnaryExp(Operator::OPT_AND, new IdExp("b")), invalid_argument);
    }

    SECTION("constructing binary expressions") {
        CHECK_NOTHROW(BinaryExp(new IdExp("a"), Operator::OPT_MUL, new IdExp("b")));
        CHECK_THROWS_AS(BinaryExp(new IdExp("c"), Operator::OPT_NOT, new IdExp("d")), invalid_argument);
    }

    SECTION("constructing assignment expression") {
        auto assignExp = make_unique<AssignExp>(new IdExp("lhs"), new LiteralExp('c'));

        CHECK(dynamic_cast<const IdExp*>(assignExp->left)->identifier == "lhs");
        CHECK(dynamic_cast<const LiteralExp*>(assignExp->right)->charVal == 'c');
    }

    SECTION("constructing function call expression") {
        CHECK_NOTHROW(CallExp(new IdExp("f"), { new LiteralExp(1), new LiteralExp(3.14) }));
        CHECK_NOTHROW(CallExp(new IdExp("f"), {}));
        CHECK_THROWS_AS(CallExp(nullptr, { new LiteralExp(0) }), invalid_argument);
        CHECK_THROWS_AS(CallExp(new IdExp("f"), { new LiteralExp(0), nullptr }), invalid_argument);
    }
}

#include <catch2/catch.hpp>
#include <memory>
#include "ast.hpp"

using namespace std;
using namespace AST;


SCENARIO("symbol table works", "[ast-scope]") {

    GIVEN("some symbol tables and types") {
        auto intType = make_type<PrimitiveType>(TYPE_INT);
        auto charType = make_type<PrimitiveType>(TYPE_CHAR);
        auto globalScope = make_shared<SymbolTable>(nullptr);
        auto funcScope = make_shared<SymbolTable>(globalScope);
        auto blockScope = make_shared<SymbolTable>(funcScope);

        globalScope->add("id1", intType);
        REQUIRE(*globalScope->getType("id1") == *intType);
        REQUIRE(globalScope->getType("id2") == nullptr);
        CHECK(globalScope->size() == 1);
        CHECK(blockScope->isLowerThan(blockScope));
        CHECK(blockScope->isLowerThan(funcScope));
        CHECK(blockScope->isLowerThan(globalScope));
        CHECK_FALSE(funcScope->isLowerThan(blockScope));

        WHEN("adding a new symbol") {
            funcScope->add("id2", charType);

            THEN("the new symbol can be found in the current scope") {
                REQUIRE(*funcScope->getType("id2") == *charType);
            }

            THEN("the new symbol cannot be found in the upper scope") {
                CHECK(globalScope->getType("id2") == nullptr);
            }

            THEN("the new symbol can be found in the sub scope") {
                CHECK(*blockScope->getType("id2") == *charType);
            }

            THEN("size of current scope is increased by 1") {
                REQUIRE(funcScope->size() == 2);
            }

            THEN("size of parent scope doesn't change") {
                REQUIRE(globalScope->size() == 1);
            }

            THEN("size of sub scope is increased by 1") {
                REQUIRE(blockScope->size() == 2);
            }
        }
    }
}


TEST_CASE("AST nodes for expressions can be constructed", "[ast-exp]") {

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

Exp * createExp() {
    return new IdExp("a"); 
}

Stmt * createStmt() {
    return new ExpStmt(new AssignExp(new IdExp("b"), new LiteralExp(0)));
}

TEST_CASE("AST nodes for statements can be constructed and destroyed", "[ast-stmt]") {

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


TEST_CASE("scopes are set up properly", "[ast-stmt]") {
    auto scope = make_shared<SymbolTable>(nullptr);

    SECTION("checking scope for if-statement") {
        auto exp = createExp();
        auto stmt1 = createStmt();
        auto stmt2 = createStmt();
        IfStmt ifStmt(exp, stmt1, stmt2);
        ifStmt.setScope(scope);

        CHECK(exp->scope == scope);
        CHECK(stmt1->scope == scope);
        CHECK(stmt2->scope == scope);
    }

    SECTION("checking scope for compound statement") {
        auto stmt1 = createStmt();
        auto stmt2 = createStmt();
        CompoundStmt compStmt({}, { stmt1, stmt2 });
        compStmt.setScope(scope);

        CHECK(stmt1->scope->isLowerThan(scope));
        CHECK(stmt1->scope != scope);
    }

    SECTION("checking scope for function definition") {
        auto compStmt = new CompoundStmt({}, {});
        auto specifier = new PrimitiveSpecifier("int");
        auto param = new ParamDec(new PrimitiveSpecifier("char"), new VarDec("a"));
        auto funcDec = new FunDec("foo", { param });
        FunDef func(specifier, funcDec, compStmt);
        func.setScope(scope);

        CHECK(func.scope->isLowerThan(scope));
        CHECK(func.scope != scope);
        CHECK(func.body->scope->isLowerThan(func.declarator->scope));
        CHECK_FALSE(func.specifier->scope->isLowerThan(func.body->scope));
    }
}

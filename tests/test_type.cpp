#include <catch2/catch.hpp>
#include <memory>
#include "symbol_table.hpp"
#include "type.hpp"

using namespace std;
using namespace smt;


TEST_CASE("types are comparable", "[ast-type]") {
    auto charType = makeType<PrimitiveType>(TYPE_CHAR);
    auto intType = makeType<PrimitiveType>(TYPE_INT);

    SECTION("comparing primitive types") {
        CHECK(*charType != *intType);
    }

    SECTION("comparing 1D arrays") {
        auto charArrT1 = makeType<ArrayType>(charType, 5);
        auto charArrT2 = makeType<ArrayType>(charType, 5);
        auto intArrT1 = makeType<ArrayType>(intType, 5);
        auto intArrT2 = makeType<ArrayType>(intType, 6);
        CHECK(as<ArrayType>(charArrT1) == as<ArrayType>(charArrT2));
        CHECK(*charArrT1 == *charArrT2);
        CHECK(*charArrT1 != *intArrT1);
        CHECK(*intArrT1 != *intArrT2);
    }

    SECTION("constructing 2D arrays") {
        auto char1dArrT1 = makeType<ArrayType>(charType, 5);
        auto char1dArrT2 = makeType<ArrayType>(charType, 5);
        auto char2dArrT1 = makeType<ArrayType>(char1dArrT1, 3);

        REQUIRE(as<ArrayType>(char2dArrT1).size == 3);
        REQUIRE(*as<ArrayType>(char2dArrT1).baseType == *char1dArrT2);

        SECTION("comparing 2D arrays") {
            auto char2dArrT2 = makeType<ArrayType>(char1dArrT2, 3);
            auto char2dArrT3 = makeType<ArrayType>(char1dArrT1, 4);
            CHECK(*char2dArrT1 == *char2dArrT2);
            CHECK(*char2dArrT2 != *char2dArrT3);
        }
    }

    SECTION("comparing structure types") {
        auto charField = make_pair(charType, string("field1"));
        auto intField = make_pair(intType, string("field2"));
        auto s1 = make_unique<StructType>(std::initializer_list { charField, intField });

        CHECK(*s1->getFieldType("field1") == *charType);

        SECTION("structures matter") {
            StructType s2({ intField, charField });
            CHECK(*s1 != s2);
        }

        SECTION("filed names don't matter") {
            StructType s2({
                make_pair(charType, "attr1"),
                make_pair(intType, "attr2")
            });
            CHECK(*s1 == s2);
        }
    }

    SECTION("comparing function types") {
        SECTION("comparing procedure types") {
            FunctionType procType1(nullptr, { charType, intType });
            FunctionType procType2(nullptr, { intType, charType });
            FunctionType procType3(nullptr, {
                makeType<PrimitiveType>(TYPE_CHAR),
                makeType<PrimitiveType>(TYPE_INT)
            });
            CHECK(procType1 != procType2);
            CHECK(procType1 == procType3);
        }

        SECTION("functions may not have any parameter") {
            FunctionType funcType1(intType, {});
            FunctionType funcType2(intType);
            FunctionType funcType3(charType);
            CHECK(funcType1 == funcType2);
            CHECK(funcType2 != funcType3);
        }
    }

    SECTION("comparing type alias") {
        auto charA1 = makeType<TypeAlias>("Alias1", charType);
        auto charA2 = makeType<TypeAlias>("Alias2", makeType<PrimitiveType>(TYPE_CHAR));
        auto charA3 = makeType<TypeAlias>("Alias3", charA1);
        auto intAlias = makeType<TypeAlias>("IntAlias", intType);

        CHECK(*charA1 == *charA2);
        CHECK(*charType == *charA2);
        CHECK(*charA2 == *charA3);
        CHECK(charA2 != intAlias);
    }
}


SCENARIO("types on the symbol table can be referenced", "[ast-scope]") {

    GIVEN("some symbol tables and types") {
        auto intType = makeType<PrimitiveType>(TYPE_INT);
        auto charType = makeType<PrimitiveType>(TYPE_CHAR);

        auto globalScope = make_shared<SymbolTable>(nullptr);
        auto funcScope = make_shared<SymbolTable>(globalScope);
        auto blockScope = make_shared<SymbolTable>(funcScope);

        WHEN("there is no data") {
            THEN("nothing can be found") {
                CHECK_FALSE(blockScope->getType("abc"));
                CHECK_FALSE(blockScope->getType("def"));
            }
        }

        WHEN("a symbol is set to null (which represents an incomplete status)") {
            funcScope->setType("a", nullptr);

            THEN("it can be accessed in the block scope") {
                auto opt = blockScope->getType("a");
                CHECK(opt);
                CHECK_FALSE(opt.value());
            }

            WHEN("a reserved symbol is referenced") {
                auto refer = blockScope->getType("a").value();
                blockScope->setType("b", refer);

                THEN("updated value can be retrived at any time") {
                    auto type = funcScope->getType("a").value();
                    type.set(new PrimitiveType(TYPE_INT));

                    auto hdl = blockScope->getType("b");
                    CHECK(hdl);
                    auto b = hdl.value();
                    CHECK(as<PrimitiveType>(b).primitive == TYPE_INT);
                }
            }
        }
    }
}


SCENARIO("symbol table works", "[ast-scope]") {

    GIVEN("some symbol tables and types") {
        auto intType = makeType<PrimitiveType>(TYPE_INT);
        auto charType = makeType<PrimitiveType>(TYPE_CHAR);

        auto globalScope = make_shared<SymbolTable>(nullptr);
        auto funcScope = make_shared<SymbolTable>(globalScope);
        auto blockScope = make_shared<SymbolTable>(funcScope);

        globalScope->setType("id1", intType);
        REQUIRE(*globalScope->getType("id1").value() == *intType);
        REQUIRE_FALSE(globalScope->getType("id2"));
        CHECK(globalScope->size() == 1);
        CHECK(blockScope->isLowerThan(blockScope));
        CHECK(blockScope->isLowerThan(funcScope));
        CHECK(blockScope->isLowerThan(globalScope));
        CHECK_FALSE(funcScope->isLowerThan(blockScope));

        WHEN("adding a new symbol") {
            funcScope->setType("id2", charType);

            THEN("the new symbol can be found in the current scope") {
                REQUIRE(*funcScope->getType("id2").value() == *charType);
            }

            THEN("the new symbol cannot be found in the upper scope") {
                CHECK_FALSE(globalScope->getType("id2"));
            }

            THEN("the new symbol can be found in the sub scope") {
                CHECK(*blockScope->getType("id2").value() == *charType);
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
#include <catch2/catch.hpp>
#include <memory>
#include "type.hpp"

using namespace std;
using namespace AST;


TEST_CASE("types are comparable", "[ast-type]") {
    shared_ptr<Type> charType(new PrimitiveType(TYPE_CHAR));
    shared_ptr<Type> intType(new PrimitiveType(TYPE_INT));

    SECTION("comparing primitive types") {
        CHECK(*charType != *intType);
    }

    SECTION("comparing 1D arrays") {
        shared_ptr<Type> charArrayType1(new ArrayType(charType, 5));
        shared_ptr<Type> charArrayType2(new ArrayType(charType, 5));
        shared_ptr<Type> intArrayType1(new ArrayType(intType, 5));
        shared_ptr<Type> intArrayType2(new ArrayType(intType, 6));
        CHECK(*charArrayType1 == *charArrayType2);
        CHECK(*charArrayType1 != *intArrayType1);
        CHECK(*intArrayType1 != *intArrayType2);
    }

    SECTION("constructing 2D arrays") {
        shared_ptr<Type> char1dArrayType1(new ArrayType(charType, 5));
        shared_ptr<Type> char1dArrayType2(new ArrayType(charType, 5));
        ArrayType char2dArrayType1(char1dArrayType1, 3);
        REQUIRE(char2dArrayType1.size == 3);
        REQUIRE(*(char2dArrayType1.baseType) == *char1dArrayType2);

        SECTION("comparing 2D arrays") {
            ArrayType char2dArrayType2(char1dArrayType2, 3);
            ArrayType char2dArrayType3(char1dArrayType1, 4);
            CHECK(char2dArrayType1 == char2dArrayType2);
            CHECK(char2dArrayType2 != char2dArrayType3);
        }
    }

    SECTION("comparing structure types") {
        auto charField = make_pair(charType, "field1");
        auto intField = make_pair(intType, "field2");
        StructType structType1({ charField, intField });

        CHECK(*structType1.getFieldType("field1") == *charType);

        SECTION("structures matter") {
            StructType structType2({ intField, charField });
            CHECK(structType1 != structType2);
        }

        SECTION("filed names don't matter") {
            StructType structType2({
                make_pair(charType, "attr1"),
                make_pair(intType, "attr2")
            });
            CHECK(structType1 == structType2);
        }
    }

    SECTION("comparing function types") {
        SECTION("comparing procedure types") {
            FunctionType procType1(nullptr, { charType, intType });
            FunctionType procType2(nullptr, { intType, charType });
            FunctionType procType3(nullptr, {
                shared_ptr<Type>(new PrimitiveType(TYPE_CHAR)),
                shared_ptr<Type>(new PrimitiveType(TYPE_INT))
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
        shared_ptr<Type> charAliasType1(new TypeAlias("Alias1", charType));
        TypeAlias charAliasType2("Alias2", shared_ptr<Type>(new PrimitiveType(TYPE_CHAR)));
        TypeAlias charAliasType3("Alias3", charAliasType1);
        TypeAlias intAliasType("IntAlias", intType);
        CHECK(*charAliasType1 == charAliasType2);
        CHECK(*charType == charAliasType2);
        CHECK(charAliasType2 == charAliasType3);
        CHECK(charAliasType2 != intAliasType);
    }
}

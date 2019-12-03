#include <catch2/catch.hpp>
#include <memory>
#include "type.hpp"

using namespace std;
using namespace AST;


SCENARIO("types are comparable", "[AST::Type]") {

    GIVEN("2 primitive types") {
        shared_ptr<Type> charType(new PrimitiveType(PrimitiveType::TYPE_CHAR));
        shared_ptr<Type> intType(new PrimitiveType(PrimitiveType::TYPE_INT));

        REQUIRE(*charType != *intType);

        WHEN("comparing some 1D arrays") {
            shared_ptr<Type> charArrayType1(new ArrayType(charType, 5));
            shared_ptr<Type> charArrayType2(new ArrayType(charType, 5));
            shared_ptr<Type> intArrayType1(new ArrayType(intType, 5));
            shared_ptr<Type> intArrayType2(new ArrayType(intType, 6));

            THEN("array types are equal only if base type match") {
                REQUIRE(*charArrayType1 == *charArrayType2);
                REQUIRE(*charArrayType1 != *intArrayType1);
            }

            THEN("array types are equal only if size match") {
                REQUIRE(*intArrayType1 != *intArrayType2);
            }
        }

        WHEN("constructing 2D arrays") {
            shared_ptr<Type> char1dArrayType1(new ArrayType(charType, 5));
            shared_ptr<Type> char1dArrayType2(new ArrayType(charType, 5));
            ArrayType char2dArrayType(char1dArrayType1, 3);

            THEN("outer dimension is correct") {
                REQUIRE(char2dArrayType.size == 3);
            }

            THEN("base type is correct") {
                REQUIRE(*char2dArrayType.baseType == *char1dArrayType2);
            }
        }
    }

}
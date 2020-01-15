#include <catch2/catch.hpp>
#include <vector>
#include <list>
#include "utils.hpp"

using namespace std;

TEST_CASE("findNull() and deleteAll() can handle single, multiple, and STL containers", "[utils]") {
    
    SECTION("checking single pointer") {
        const int * p0 = nullptr;
        int * p1 = new int;
        CHECK(hasNull(p0));
        CHECK_FALSE(hasNull(p1));

        deleteAll(p1);
        CHECK(p1 == nullptr);
    }

    SECTION("checking multiple pointers") {
        int * p0 = new int;
        int * p1 = nullptr;
        int * p2 = new int;
        CHECK_FALSE(hasNull(p0, p2));
        CHECK(hasNull(p0, p1, p2));

        deleteAll(p0, p1, p2);
        CHECK(p0 == nullptr);
        CHECK(p1 == nullptr);
        CHECK(p2 == nullptr);
    }

    SECTION("checking pointer container") {
        vector<double*> ptrs1 { new double, new double, nullptr };
        vector<int*> ptrs2 { new int, new int };
        CHECK(hasNull(ptrs1));
        CHECK_FALSE(hasNull(ptrs2));

        deleteAll(ptrs1, ptrs2);
        CHECK(ptrs1[0] == nullptr);
        CHECK(ptrs1[1] == nullptr);
        CHECK(ptrs2[0] == nullptr);
        CHECK(ptrs2[1] == nullptr);
    }

    SECTION("checking mixed situation") {
        vector<char*> ptrs { new char, new char };
        int *p0 = nullptr;
        double *p1 = new double;
        CHECK(hasNull(p1, ptrs, p0));
        CHECK_FALSE(hasNull(p1, ptrs));

        deleteAll(ptrs[0], ptrs[1]);
        CHECK(ptrs[0] == nullptr);
        CHECK(ptrs[1] == nullptr);
    }
}


struct MemFlag {
    static int cnt;
    MemFlag() {
        cnt++;
    }
    ~MemFlag() {
        cnt--;
    }
};

int MemFlag::cnt = 0;

SCENARIO("share data using Shared", "[Shared]") {
    GIVEN("a class named DestructorTester") {
        Shared<MemFlag> s1;
        Shared<MemFlag> s2 = s1;
        CHECK(MemFlag::cnt == 0);
        CHECK(s2.useCount() == 2);

        /* WHEN("setting a value") */ {
            s2.set(new MemFlag);
            CHECK(s2.value().cnt == 1);
            CHECK(s1.get() == s2.get());

            {
                auto s3 = s1;
                THEN("data are shared") {
                    CHECK(s3.get() == s2.get());
                }
                CHECK(MemFlag::cnt == 1);
                CHECK(s3.useCount() == 3);
            }
            CHECK(s1.useCount() == 2);

            int oldCnt = MemFlag::cnt;
            CHECK(oldCnt == 1);
            {
                auto newVal = new MemFlag;
                THEN("there are 2 pieces of data") {
                    CHECK(MemFlag::cnt == oldCnt + 1);
                }
                s1.set(newVal);
                THEN("only 1 piece of data exits now") {
                    CHECK(MemFlag::cnt == oldCnt);
                }
            }
            CHECK(MemFlag::cnt == oldCnt);

            {
                Shared<MemFlag> t(new MemFlag);
                CHECK(t.get() != s2.get());
                CHECK(t.useCount() == 1);
                CHECK(s1.useCount() == 2);
                CHECK(MemFlag::cnt == 2);

                s2 = t;
                CHECK(t.get() == s2.get());
                CHECK(t.useCount() == 2);
                CHECK(s1.useCount() == 1);
                CHECK(MemFlag::cnt == 2);
            }
        }
        CHECK(s2.useCount() == 1);
        CHECK(s1.get() != s2.get());
    }

    // shared object destroyed
    CHECK(MemFlag::cnt == 0);
}

// TODO: test overloaded operators

#include <catch2/catch_test_macros.hpp>

#include "pivot/graphics/PivotFlags.hxx"

using namespace pivot;

enum class TestEnum : FlagsType {
    eOne = BIT(1),
    eTwo = BIT(2),
    eThree = BIT(3),
    eFour = BIT(4),
    eFive = BIT(5),
    eSix = BIT(6),
};
using TestEnumFlags = Flags<TestEnum>;
ENABLE_FLAGS_FOR_ENUM(TestEnum)

TEST_CASE("Pivot flags test", "[flags]")
{
    TestEnumFlags f1 = TestEnum::eOne | TestEnum::eFour;
    REQUIRE(bool(f1 & TestEnum::eOne));
    REQUIRE_FALSE(bool(f1 & TestEnum::eTwo));
    REQUIRE_FALSE(bool(f1 & TestEnum::eThree));
    REQUIRE(bool(f1 & TestEnum::eFour));
    REQUIRE_FALSE(bool(f1 & TestEnum::eFive));
    REQUIRE_FALSE(bool(f1 & TestEnum::eSix));

    SECTION("Assigment Operators")
    {
        SECTION("OR")
        {
            f1 |= TestEnum::eFive;
            REQUIRE(bool(f1 & TestEnum::eOne));
            REQUIRE_FALSE(bool(f1 & TestEnum::eTwo));
            REQUIRE_FALSE(bool(f1 & TestEnum::eThree));
            REQUIRE(bool(f1 & TestEnum::eFour));
            REQUIRE(bool(f1 & TestEnum::eFive));
            REQUIRE_FALSE(bool(f1 & TestEnum::eSix));
        }

        SECTION("AND")
        {
            f1 &= TestEnum::eFour;
            REQUIRE_FALSE(bool(f1 & TestEnum::eOne));
            REQUIRE_FALSE(bool(f1 & TestEnum::eTwo));
            REQUIRE_FALSE(bool(f1 & TestEnum::eThree));
            REQUIRE(bool(f1 & TestEnum::eFour));
            REQUIRE_FALSE(bool(f1 & TestEnum::eFive));
            REQUIRE_FALSE(bool(f1 & TestEnum::eSix));
        }

        SECTION("NOT")
        {
            f1 ^= TestEnum::eFour;
            REQUIRE(bool(f1 & TestEnum::eOne));
            REQUIRE_FALSE(bool(f1 & TestEnum::eTwo));
            REQUIRE_FALSE(bool(f1 & TestEnum::eThree));
            REQUIRE_FALSE(bool(f1 & TestEnum::eFour));
            REQUIRE_FALSE(bool(f1 & TestEnum::eFive));
            REQUIRE_FALSE(bool(f1 & TestEnum::eSix));
        }

        SECTION("EQUAL") { REQUIRE(f1 == (TestEnum::eOne | TestEnum::eFour)); }
        SECTION("INEQUAL") { REQUIRE(f1 != (TestEnum::eOne)); }
    }
}

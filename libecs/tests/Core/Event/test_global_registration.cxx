#include <pivot/ecs/Core/Event/index.hxx>

#include <catch2/catch_test_macros.hpp>

using namespace pivot::ecs::event;

TEST_CASE("Test registration of event index", "[description][registration]")
{
    GlobalIndex eventIndexForRegistrationTest;

    SECTION("Register same event in Global Index", "[description][registration]")
    {
        Description description{
            .name = "Duplicate",
        };
        REQUIRE_NOTHROW(eventIndexForRegistrationTest.registerEvent(description));
        REQUIRE_THROWS_AS(eventIndexForRegistrationTest.registerEvent(description), Index::DuplicateError);
    }

    SECTION("Register valid event in Global Index", "[description][registration]")
    {
        Description description{
            .name = "Valid",
            .entities =
                {
                    "Oui",
                },
            .payload = pivot::ecs::data::BasicType::Boolean,
        };
        REQUIRE_NOTHROW(eventIndexForRegistrationTest.registerEvent(description));
        SECTION("Get event in Global Index", "[description][registration]")
        {
            REQUIRE(eventIndexForRegistrationTest.getDescription("Valid").has_value());
        }
    }

    SECTION("Register invalid event in Global Index", "[description][registration]")
    {
        Description description;
        REQUIRE_THROWS_AS(eventIndexForRegistrationTest.registerEvent(description), Description::ValidationError);
    }

    SECTION("Get not registered event in Global Index", "[description][registration]")
    {
        REQUIRE(!eventIndexForRegistrationTest.getDescription("Yolo").has_value());
    }

    SECTION("Iterator of event Global Index", "[description][registration]")
    {
        for (auto &[name, description]: eventIndexForRegistrationTest) {
            REQUIRE((name == "Valid" || name == "Duplicate"));
        }
    }
}

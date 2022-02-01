#include <pivot/ecs/Core/Event/index.hxx>
#include <catch2/catch.hpp>

using namespace pivot::ecs::event;

GlobalIndex eventIndexForRegistrationTest;

TEST_CASE("Register same event in Global Index", "[description][registration]")
{
    Description description{
        .name = "Duplicate",
    };
    REQUIRE_NOTHROW(eventIndexForRegistrationTest.registerEvent(description));
    REQUIRE_THROWS_AS(eventIndexForRegistrationTest.registerEvent(description), Index::DuplicateError);
}

TEST_CASE("Register valid event in Global Index", "[description][registration]")
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
}

TEST_CASE("Register invalid event in Global Index", "[description][registration]")
{
    Description description;
    REQUIRE_THROWS_AS(eventIndexForRegistrationTest.registerEvent(description), Description::ValidationError);
}

TEST_CASE("Get event in Global Index", "[description][registration]")
{
    REQUIRE(eventIndexForRegistrationTest.getDescription("Valid").has_value());
}

TEST_CASE("Get not registered event in Global Index", "[description][registration]")
{
    REQUIRE(!eventIndexForRegistrationTest.getDescription("Yolo").has_value());
}

TEST_CASE("Iterator of event Global Index", "[description][registration]")
{
    for (auto &[name, description]: eventIndexForRegistrationTest) { REQUIRE((name == "Valid" || name == "Duplicate")); }
}

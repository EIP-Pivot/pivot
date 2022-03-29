#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

using namespace pivot::ecs;

#include <catch2/catch_test_macros.hpp>

void test_global_registration(const systems::Description &, component::ArrayCombination &,
                              const event::EventWithComponent &)
{
}

TEST_CASE("Test system index registration", "[description][registration]")
{
    systems::GlobalIndex indexForRegistrationTest;
    SECTION("Register same system in Global Index", "[description][registration]")
    {
        event::Description event{
            .name = "Colid",
            .entities = {},
            .payload = pivot::ecs::data::BasicType::Number,
        };
        systems::Description description{
            .name = "Duplicate",
            .systemComponents =
                {
                    "RigidBody",
                    "Tag",
                },
            .eventListener = event,
            .system = &test_global_registration,
        };
        REQUIRE_NOTHROW(indexForRegistrationTest.registerSystem(description));
        REQUIRE_THROWS_AS(indexForRegistrationTest.registerSystem(description), systems::Index::DuplicateError);
    }

    SECTION("Register valid system in Global Index", "[description][registration]")
    {
        event::Description event{
            .name = "Colid",
            .entities = {},
            .payload = pivot::ecs::data::BasicType::Number,
        };
        systems::Description description{
            .name = "Valid",
            .systemComponents =
                {
                    "RigidBody",
                    "Tag",
                },
            .eventListener = event,
            .system = &test_global_registration,
        };
        REQUIRE_NOTHROW(indexForRegistrationTest.registerSystem(description));
        SECTION("Get system in Global Index", "[description][registration]")
        {
            REQUIRE(indexForRegistrationTest.getDescription("Valid").has_value());
        }
    }

    SECTION("Register invalid system in Global Index", "[description][registration]")
    {
        systems::Description description;
        REQUIRE_THROWS_AS(indexForRegistrationTest.registerSystem(description), systems::Description::ValidationError);
    }

    SECTION("Get not registered system in Global Index", "[description][registration]")
    {
        REQUIRE(!indexForRegistrationTest.getDescription("Yolo").has_value());
    }

    SECTION("Iterator of Global Index", "[description][registration]")
    {
        for (auto &[name, description]: indexForRegistrationTest) { REQUIRE((name == "Valid" || name == "Duplicate")); }
    }
}

#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

using namespace pivot::ecs;

systems::GlobalIndex indexForRegistrationTest;

void test_global_registration(const systems::Description::availableEntities &, const systems::Description &,
                              const systems::Description::systemArgs &, const event::Event &)
{
}

TEST_CASE("Register same system in Global Index", "[description][registration]")
{
    systems::Description description{
        .name = "Duplicate",
        .components =
            {
                "RigidBody",
                "Tag",
            },
        .system = &test_global_registration,
    };
    REQUIRE_NOTHROW(indexForRegistrationTest.registerSystem(description));
    REQUIRE_THROWS_AS(indexForRegistrationTest.registerSystem(description), systems::Index::DuplicateError);
}

TEST_CASE("Register valid system in Global Index", "[description][registration]")
{
    systems::Description description{
        .name = "Valid",
        .components =
            {
                "RigidBody",
                "Tag",
            },
        .system = &test_global_registration,
    };
    REQUIRE_NOTHROW(indexForRegistrationTest.registerSystem(description));
}

TEST_CASE("Register invalid system in Global Index", "[description][registration]")
{
    systems::Description description;
    REQUIRE_THROWS_AS(indexForRegistrationTest.registerSystem(description), systems::Description::ValidationError);
}

TEST_CASE("Get system in Global Index", "[description][registration]")
{
    REQUIRE(indexForRegistrationTest.getDescription("Valid").has_value());
}

TEST_CASE("Get not registered system in Global Index", "[description][registration]")
{
    REQUIRE(!indexForRegistrationTest.getDescription("Yolo").has_value());
}

TEST_CASE("Iterator of Global Index", "[description][registration]")
{
    for (auto &[name, description]: indexForRegistrationTest) { REQUIRE((name == "Valid" || name == "Duplicate")); }
}

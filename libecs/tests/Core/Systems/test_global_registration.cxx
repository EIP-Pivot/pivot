#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

using namespace pivot::ecs::systems;

GlobalIndex indexForRegistrationTest;

void test_global_registration(std::vector<std::vector<std::any>> components) {}

TEST_CASE("Register same system in Global Index", "[description][registration]")
{
    Description description{
        .name = "Duplicate",
        .arguments =
            {
                "RigidBody",
                "Tag",
            },
        .system = &test_global_registration,
    };
    REQUIRE_NOTHROW(indexForRegistrationTest.registerSystem(description));
    REQUIRE_THROWS_AS(indexForRegistrationTest.registerSystem(description), Index::DuplicateError);
}

TEST_CASE("Register valid system in Global Index", "[description][registration]")
{
    Description description{
        .name = "Valid",
        .arguments =
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
    Description description;
    REQUIRE_THROWS_AS(indexForRegistrationTest.registerSystem(description),
                      Description::ValidationError);
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

#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

using namespace pivot::ecs::systems;

void test_global_registration(RigidBody, Tag, Gravity) {}

TEST_CASE("Register same system in Global Index", "[systems]")
{
    Description description = Description::build_system_description("Duplicate", &test_global_registration);
    REQUIRE_NOTHROW(GlobalIndex::getSingleton().registerSystem(description, &test_global_registration));
    REQUIRE_THROWS_AS(GlobalIndex::getSingleton().registerSystem(description, &test_global_registration), Index::DuplicateError);
}

TEST_CASE("Register valid system in Global Index", "[systems]")
{
    Description description = Description::build_system_description("Valid", &test_global_registration);
    REQUIRE_NOTHROW(GlobalIndex::getSingleton().registerSystem(description, &test_global_registration));
}

TEST_CASE("Register invalid system in Global Index", "[systems]")
{
    Description description;
    REQUIRE_THROWS_AS(GlobalIndex::getSingleton().registerSystem(description, &test_global_registration),
                      Description::ValidationError);
}

TEST_CASE("Get system in Global Index", "[systems]")
{
    REQUIRE(GlobalIndex::getSingleton().getDescription("Valid").has_value());
}

TEST_CASE("Get not registered system in Global Index", "[systems]")
{
    REQUIRE(!GlobalIndex::getSingleton().getDescription("Yolo").has_value());
}

TEST_CASE("Iterator of Global Index", "[systems]")
{
    for (auto &[name, description]: GlobalIndex::getSingleton()) { REQUIRE((name == "Valid" || name == "Duplicate")); }
}

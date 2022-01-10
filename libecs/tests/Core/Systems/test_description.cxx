#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Systems/description.hxx>

#include <iostream>

using namespace pivot::ecs::systems;

void coucou(RigidBody, Tag, Gravity) {}

TEST_CASE("valid system description", "[systems]")
{
    const std::string test = "Control";
    Description description = Description::build_system_description(test, &coucou);
    REQUIRE_NOTHROW(description.validate());
}

TEST_CASE("Empty system description", "[systems]")
{
    Description description;
    REQUIRE_THROWS_WITH(description.validate(), "Empty system name");
}

TEST_CASE("Empty args system description", "[systems]")
{
    Description description{
        .name = "Invalid",
    };
    REQUIRE_THROWS_WITH(description.validate(), "Empty system argument");
}

TEST_CASE("Invalid args system description", "[systems]")
{
    Description description{
        .name = "Invalid",
        .arguments = {"NOT REGISTERED"},
    };
    REQUIRE_THROWS_WITH(description.validate(), Catch::Matchers::Contains("Component ") && Catch::Matchers::Contains(" his not registered."));
}
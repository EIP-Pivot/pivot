#include <catch2/catch.hpp>
#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>
#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Components/Transform.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs::component;

TEST_CASE("all components are registered globally", "[component]")
{
    REQUIRE(GlobalIndex::getSingleton().getDescription("Tag").has_value());
    REQUIRE(GlobalIndex::getSingleton().getDescription("Gravity").has_value());
    REQUIRE(GlobalIndex::getSingleton().getDescription("RigidBody").has_value());
    REQUIRE(GlobalIndex::getSingleton().getDescription("Transform").has_value());
}

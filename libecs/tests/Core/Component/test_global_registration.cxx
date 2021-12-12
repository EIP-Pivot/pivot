#include <catch2/catch.hpp>
#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>
#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Components/Transform.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs::component;

TEST_CASE("all components are registered globally", "[component]")
{
    REQUIRE(GlobalIndex::getSingleton().getComponentNameByType<Tag>() == "Tag");
    REQUIRE(GlobalIndex::getSingleton().getDescription("Tag").has_value());

    REQUIRE(GlobalIndex::getSingleton().getComponentNameByType<Gravity>() == "Gravity");
    REQUIRE(GlobalIndex::getSingleton().getDescription("Gravity").has_value());

    REQUIRE(GlobalIndex::getSingleton().getComponentNameByType<RigidBody>() == "RigidBody");
    REQUIRE(GlobalIndex::getSingleton().getDescription("RigidBody").has_value());

    REQUIRE(GlobalIndex::getSingleton().getComponentNameByType<pivot::ecs::component::Transform>() == "Transform");
    REQUIRE(GlobalIndex::getSingleton().getDescription("Transform").has_value());
}

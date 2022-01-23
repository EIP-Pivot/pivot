#include <catch2/catch.hpp>

#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs::component;

TEST_CASE("Dense component arrays", "[component][scene]")
{
    auto description = GlobalIndex::getSingleton().getDescription("Tag").value();

    DenseTypedComponentArray<Tag> array(description);

    REQUIRE(array.getData().size() == 0);
    REQUIRE(!array.getValueForEntity(0).has_value());

    array.setValueForEntity(0, std::make_any<Tag>("test"));
    REQUIRE(array.getData().size() == 1);
    REQUIRE(array.getValueForEntity(0).has_value());
    REQUIRE(std::any_cast<Tag>(array.getValueForEntity(0).value()).name == "test");

    array.setValueForEntity(1000, std::make_any<Tag>("test"));
    REQUIRE(array.getData().size() == 1001);
    REQUIRE(array.getValueForEntity(1000).has_value());
    REQUIRE(!array.getValueForEntity(500).has_value());

    array.setValueForEntity(1000000, std::nullopt);
    REQUIRE(array.getData().size() == 1001);
}

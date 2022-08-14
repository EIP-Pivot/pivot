#include <catch2/catch_test_macros.hpp>

#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs;
using namespace pivot::ecs::component;
using namespace pivot::ecs::data;

TEST_CASE("Dense component arrays", "[component][scene]")
{
    auto description = Tag::description;

    DenseTypedComponentArray<Tag> array(description);

    REQUIRE(array.getData().size() == 0);
    REQUIRE(!array.getValueForEntity(0).has_value());

    array.setValueForEntity(0, Value{Record{{"name", "test"}}});
    REQUIRE(array.getData().size() == 1);
    REQUIRE(array.getValueForEntity(0).has_value());
    REQUIRE(array.getValueForEntity(0).value() == Value{Record{{"name", "test"}}});

    array.setValueForEntity(1000, Value{Record{{"name", "test"}}});
    REQUIRE(array.getData().size() == 1001);
    REQUIRE(array.getValueForEntity(1000).has_value());
    REQUIRE(!array.getValueForEntity(500).has_value());

    array.setValueForEntity(1000000, std::nullopt);
    REQUIRE(array.getData().size() == 1001);
}

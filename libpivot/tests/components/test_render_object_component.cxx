#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Data/value_serialization.hxx>
#include <pivot/graphics/types/RenderObject.hxx>

using namespace nlohmann;
using namespace pivot::ecs::data;
using namespace pivot::ecs::component;

TEST_CASE("transform adapter work", "[graphics]")
{
    Transform transform;

    Value transform_value = json::parse(R"({"scale": [1,2,3], "position": [5, 5, 5], "rotation": [0, 2.7, 0.4]})");
    helpers::Helpers<Transform>::updateTypeWithValue(transform, transform_value);

    Transform expected;
    expected.position = {5, 5, 5};
    expected.rotation = {0, 2.7, 0.4};
    expected.scale = {1, 2, 3};
    Value expected_value = helpers::Helpers<Transform>::createValueFromType(expected);

    REQUIRE(transform_value.type() == expected_value.type());
    REQUIRE(transform_value == expected_value);
    REQUIRE(transform == expected);
}

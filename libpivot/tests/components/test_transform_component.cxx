#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <pivot/builtins/components/Transform.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Data/value_serialization.hxx>

using namespace nlohmann;
using namespace pivot::ecs::data;
using namespace pivot::ecs::component;
using namespace pivot::builtins::components;

TEST_CASE("Transform component works", "[graphics][component]")
{
    auto description = Transform::description;
    auto array = description.createContainer(description);

    const auto data = R"({"scale": [1,2,3], "position": [5, 5, 5], "rotation": [0, 2.7, 0.4], "root": {"entity": 42}})";

    REQUIRE_NOTHROW(array->setValueForEntity(0, json::parse(data).get<Value>()));

    const pivot::graphics::Transform expected{{5, 5, 5}, {0, 2.7, 0.4}, {1, 2, 3}, {42}};
    const pivot::graphics::Transform &transform =
        dynamic_cast<DenseTypedComponentArray<pivot::graphics::Transform> &>(*array).getData().front();
    REQUIRE(transform == expected);

    REQUIRE(description.defaultValue == Value{Record{{"position", Value{glm::vec3{0, 0, 0}}},
                                                     {"rotation", Value{glm::vec3{0, 0, 0}}},
                                                     {"scale", Value{glm::vec3{1, 1, 1}}},
                                                     {"root", Value{pivot::EntityRef::empty()}}}});
}

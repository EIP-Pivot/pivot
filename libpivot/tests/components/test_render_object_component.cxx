#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <pivot/builtins/components/RenderObject.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Data/value_serialization.hxx>

using namespace nlohmann;
using namespace pivot::ecs::data;
using namespace pivot::ecs::component;
using namespace pivot::builtins::components;
using Transform = pivot::graphics::Transform;

TEST_CASE("transform adapter work", "[graphics]")
{
    Transform transform;

    Value transform_value = json::parse(R"({"scale": [1,2,3], "position": [5, 5, 5], "rotation": [0, 2.7, 0.4]})");
    helpers::Helpers<Transform>::updateTypeWithValue(transform, transform_value);

    Transform expected{{5, 5, 5}, {0, 2.7, 0.4}, {1, 2, 3}};
    Value expected_value = helpers::Helpers<Transform>::createValueFromType(expected);

    REQUIRE(transform_value.type() == expected_value.type());
    REQUIRE(transform_value == expected_value);
    REQUIRE(transform == expected);
}

TEST_CASE("RenderObject component works", "[graphics][component]")
{
    auto description = RenderObject::description;
    auto array = description.createContainer(description);

    const auto data =
        R"({"meshID": "sponza","pipelineID":"default", "materialIndex":"blue","transform":{"scale": [1,2,3], "position": [5, 5, 5], "rotation": [0, 2.7, 0.4]}})";

    REQUIRE_NOTHROW(array->setValueForEntity(0, json::parse(data).get<Value>()));

    const RenderObject expected{"sponza", "default", "blue", {{5, 5, 5}, {0, 2.7, 0.4}, {1, 2, 3}}

    };
    const RenderObject &renderObject = dynamic_cast<DenseTypedComponentArray<RenderObject> &>(*array).getData().front();
    REQUIRE(renderObject == expected);

    REQUIRE(description.defaultValue ==
            Value{Record{{"meshID", Value{"cube"}},
                         {"pipelineID", Value{""}},
                         {"materialIndex", Value{"white"}},
                         {"transform", Value{Record{{"position", Value{glm::vec3{0, 0, 0}}},
                                                    {"rotation", Value{glm::vec3{0, 0, 0}}},
                                                    {"scale", Value{glm::vec3{1, 1, 1}}}}}}}});
}

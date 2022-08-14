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

TEST_CASE("RenderObject component works", "[graphics][component]")
{
    auto description = RenderObject::description;
    auto array = description.createContainer(description);

    const auto data =
        R"(
{
  "meshID": {
    "asset": {
      "name": "sponza"
    }
  },
  "pipelineID": "default",
  "materialIndex": {
    "asset": {
      "name": "blue"
    }
  }
}
)";

    REQUIRE_NOTHROW(array->setValueForEntity(0, json::parse(data).get<Value>()));

    const pivot::graphics::RenderObject expected{"sponza", "default", "blue"};
    const pivot::graphics::RenderObject &renderObject =
        dynamic_cast<DenseTypedComponentArray<pivot::graphics::RenderObject> &>(*array).getData().front();
    REQUIRE(renderObject == expected);

    REQUIRE(description.defaultValue ==
            Value{Record{{"meshID", Asset{"cube"}}, {"pipelineID", Value{""}}, {"materialIndex", Asset{""}}}});
}

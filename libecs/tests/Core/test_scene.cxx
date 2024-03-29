#include <boost/fusion/include/adapt_struct.hpp>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/description_helpers_impl.hxx>
#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Scene.hxx>

#include <catch2/catch_test_macros.hpp>

using namespace pivot::ecs::component;
using namespace pivot::ecs::data;
using namespace pivot::ecs;

struct TestComponent {
    int data;
};

BOOST_FUSION_ADAPT_STRUCT(TestComponent, data);

namespace pivot::ecs::component::helpers
{
template <>
constexpr const char *component_name<TestComponent> = "TestComponent";
}

static const Description description =
    helpers::build_component_description<TestComponent, DenseTypedComponentArray<TestComponent>>("TestComponent");

TEST_CASE("A scene can register components and add entities", "[component][scene]")
{
    const auto sceneName = "Test scene";
    Scene scene{sceneName};

    REQUIRE(scene.getName() == sceneName);
    const auto newSceneName = "New scene name";
    scene.setName(newSceneName);
    REQUIRE(scene.getName() == newSceneName);

    auto &cm = scene.getComponentManager();

    REQUIRE(cm.GetComponentId("Tag").has_value());

    REQUIRE(!cm.GetComponentId("TestComponent").has_value());
    cm.RegisterComponent(description);
    REQUIRE(cm.GetComponentId("TestComponent").has_value());
    auto test_component_id = cm.GetComponentId("TestComponent").value();

    const auto entityName = "Test entity";
    Entity entity;
    REQUIRE_NOTHROW(entity = scene.CreateEntity(entityName));
    REQUIRE(scene.getEntityName(entity) == entityName);

    REQUIRE(!cm.GetComponent(entity, test_component_id).has_value());
    cm.AddComponent(entity, Value{Record{{"data", 42}}}, test_component_id);
    auto test_component_value = cm.GetComponent(entity, test_component_id);
    REQUIRE(test_component_value.has_value());
    REQUIRE(test_component_value.value() == Value{Record{{"data", 42}}});

    const auto newEntityName = "Test entity modified";
    for (auto component: cm.GetAllComponents(entity)) {
        const Description &description = component.description();
        REQUIRE((description.name == "TestComponent" || description.name == "Tag"));
        if (description.name == "Tag") { component.set(Value{Record{{"name", newEntityName}}}); }
    }
    REQUIRE(scene.getEntityName(entity) == newEntityName);
    REQUIRE(scene.getEntityID(newEntityName) == entity);
}

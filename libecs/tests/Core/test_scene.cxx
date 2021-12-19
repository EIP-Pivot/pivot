#include <boost/fusion/include/adapt_struct.hpp>
#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Scene.hxx>

template pivot::ecs::component::Manager *std::unique_ptr<pivot::ecs::component::Manager>::get() const;
template EntityManager *std::unique_ptr<EntityManager>::get() const;
template EventManager *std::unique_ptr<EventManager>::get() const;
template SystemManager *std::unique_ptr<SystemManager>::get() const;

using namespace pivot::ecs::component;

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
    helpers::build_component_description<TestComponent, DenseTypedComponentArray<TestComponent>>("TestComponent",
                                                                                                 false);

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
    for (auto &[name, description]: GlobalIndex::getSingleton()) { REQUIRE(cm.GetComponentId(name).has_value()); }

    REQUIRE(!cm.GetComponentId("TestComponent").has_value());
    cm.RegisterComponent(description);
    REQUIRE(cm.GetComponentId("TestComponent").has_value());

    const auto entityName = "Test entity";
    Entity entity;
    REQUIRE_NOTHROW(entity = scene.CreateEntity(entityName));
    // REQUIRE(scene.getEntityName(entity) == entityName);
}

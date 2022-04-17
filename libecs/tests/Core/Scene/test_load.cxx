#include "pivot/ecs/Core/Scene.hxx"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>
#include <pivot/ecs/Components/Tag.hxx>

using namespace nlohmann;
using namespace pivot::ecs::data;

void test_load_scene_system(const pivot::ecs::systems::Description &, pivot::ecs::component::ArrayCombination &,
                            const pivot::ecs::event::EventWithComponent &)
{
}

TEST_CASE("Load the scene", "[Scene][Load]")
{
    json obj = json::parse(
        R"({"components": [{"Gravity": {"force": [0.0,0.0,0.0]},"RigidBody": {"acceleration": [0.0,0.0,0.0],"velocity": [0.0,0.0,0.0]},"Tag": {"name": "yolo"}}, {"Gravity": {"force": [0.0,0.0,0.0]},"RigidBody": {"acceleration": [0.0,0.0,0.0],"velocity": [0.0,0.0,0.0]},"Tag": {"name": "alloy"}}],"name": "Default","systems":["Test Description"]})");

    pivot::ecs::component::Index cIndex;
    pivot::ecs::systems::Index sIndex;
    cIndex.registerComponent(Gravity::description);
    cIndex.registerComponent(RigidBody::description);
    cIndex.registerComponent(Tag::description);

    pivot::ecs::event::Description event{
        .name = "Colid",
        .entities = {},
        .payload = BasicType::Number,
    };
    pivot::ecs::systems::Description description{
        .name = "Test Description",
        .systemComponents =
            {
                "RigidBody",
                "Tag",
            },
        .eventListener = event,
        .system = &test_load_scene_system,
    };
    sIndex.registerSystem(description);
    Scene LaS = Scene::load(obj, cIndex, sIndex);

    REQUIRE(LaS.getLivingEntityCount() == 2);

    auto &cManager = LaS.getComponentManager();
    auto tagId = cManager.GetComponentId("Tag");
    auto GravityId = cManager.GetComponentId("Gravity");
    auto RigidBodyId = cManager.GetComponentId("RigidBody");

    REQUIRE(cManager.GetComponent(0, tagId.value()).has_value());
    REQUIRE(cManager.GetComponent(0, GravityId.value()).has_value());
    REQUIRE(cManager.GetComponent(0, RigidBodyId.value()).has_value());

    auto &sManager = LaS.getSystemManager();
    for (auto [name, _]: sManager) { REQUIRE(name == "Test Description"); }
}
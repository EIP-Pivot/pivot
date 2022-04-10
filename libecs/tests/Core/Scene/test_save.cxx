#include <catch2/catch_test_macros.hpp>

#include <pivot/ecs/Core/Scene.hxx>

TEST_CASE("Test save scene", "[Scene][save]")
{
    Scene scene;
    scene.setName("test");

    auto cManager = scene.getComponentManager();
    auto tagID = cManager.GetComponentId("Tag").value();
    Entity e = scene.CreateEntity();
    cManager.AddComponent(e, pivot::ecs::data::Value{pivot::ecs::data::Record{{"name", "test"}}}, tagID);
}
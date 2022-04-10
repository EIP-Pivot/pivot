#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <iostream>
#include <pivot/ecs/Core/Scene.hxx>

using namespace pivot::ecs;

void test_save_scene_system(const systems::Description &, component::ArrayCombination &,
                            const event::EventWithComponent &)
{
}

TEST_CASE("Test save scene", "[Scene][save]")
{
    Scene scene("test");

    auto &cManager = scene.getComponentManager();
    Entity e = scene.CreateEntity("test");

    event::Description event{
        .name = "Colid",
        .entities = {},
        .payload = data::BasicType::Number,
    };
    systems::Description description{
        .name = "Test Description",
        .systemComponents =
            {
                "RigidBody",
                "Tag",
            },
        .eventListener = event,
        .system = &test_save_scene_system,
    };
    systems::Description description2{
        .name = "Test Description 2",
        .systemComponents =
            {
                "RigidBody",
                "Tag",
            },
        .eventListener = event,
        .system = &test_save_scene_system,
    };
    systems::GlobalIndex::getSingleton().registerSystem(description);
    systems::GlobalIndex::getSingleton().registerSystem(description2);
    scene.getSystemManager().useSystem(description);
    scene.getSystemManager().useSystem(description2);
    scene.save("test/save.json");
    REQUIRE(std::filesystem::exists("test/save.json"));
    // read file
    std::string output;
    std::string line;
    std::ifstream save("test/save.json");
    while (std::getline(save, line)) { output += line; }
    save.close();
    REQUIRE("{\"components\":[{\"Tag\":{\"name\":\"test\"}}],\"name\":\"test\",\"systems\":[\"Test "
            "Description\",\"Test Description 2\"]}" == to_string(nlohmann::json::parse(output)));
}
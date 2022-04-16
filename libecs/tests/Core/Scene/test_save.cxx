#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <pivot/ecs/Core/Scene.hxx>

#include <pivot/ecs/Components/RigidBody.hxx>

using namespace pivot::ecs;

void test_save_scene_system(const systems::Description &, component::ArrayCombination &,
                            const event::EventWithComponent &)
{
}

TEST_CASE("Test save scene", "[Scene][save]")
{
    Scene scene("test");

    auto &cManager = scene.getComponentManager();
    cManager.RegisterComponent(RigidBody::description);
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
    scene.getSystemManager().useSystem(description);
    scene.getSystemManager().useSystem(description2);
    if (!std::filesystem::exists("test")) { std::filesystem::create_directory("test"); }
    scene.save("test/save.json");
    REQUIRE(std::filesystem::exists("test/save.json"));
    // read file
    std::string output;
    std::string line;
    std::ifstream save("test/save.json");
    while (std::getline(save, line)) { output += line; }
    save.close();
    REQUIRE(
        nlohmann::json::parse(
            R"({"components":[{"Tag":{"name":"test"}}],"name":"test","systems":["Test Description","Test Description 2"]})") ==
        nlohmann::json::parse(output));
}
#include "pivot/ecs/Core/SceneManager.hxx"
#include "pivot/script/Engine.hxx"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

using namespace pivot::ecs;

TEST_CASE("Scripting-Interpreter-LoadScene")
{
    std::cout << "------Interpreter LoadScene------start (lambda must be the same as in engine.cxx)" << std::endl;

    component::Index cind;
    systems::Index sind;
    pivot::ecs::SceneManager scenemgr;
    // Register 2 scenes, and set Scene2 as current active scene
    scenemgr.registerScene("Scene1");
    scenemgr.registerScene("Scene2");
    scenemgr.setCurrentSceneId(scenemgr.getSceneId("Scene2").value());
    script::Engine engine(sind, cind,
                          pivot::ecs::script::interpreter::builtins::BuiltinContext{
                              [](auto) { return false; },
                              [&scenemgr](const std::string &scene) {
                                  // Check that the scene is loaded
                                  if (!scenemgr.getSceneId(scene).has_value()) {
                                      logger.warn("loadScene") << "The scene " << scene << " doesn't exist.";
                                      return;
                                  }
                                  // Set the scene as the active one
                                  scenemgr.setCurrentSceneId(scenemgr.getSceneId(scene).value());
                              }});
    // std::string file = "C:/Users/Najo/eip/pivot/libscript/tests/loadScene.pvt";
    // engine.loadFile(file);
    std::string fileContent = "component C\n"
                              "\tString str\n"
                              "system S(anyEntity<C>) event Tick(Number deltaTime)\n"
                              "\tloadScene(\"Scene1\")\n";
    engine.loadFile(fileContent, true);

    REQUIRE(sind.getDescription("S").has_value());
    REQUIRE(cind.getDescription("C").has_value());

    auto Cdescription = cind.getDescription("C").value();
    auto Sdescription = sind.getDescription("S").value();
    auto array1 = Cdescription.createContainer(Cdescription);
    std::vector<data::Value> entity = {data::Record{{"str", "foo"}}};
    array1->setValueForEntity(0, entity.at(0));
    component::ArrayCombination combinations{{std::ref(*array1)}};
    event::EventWithComponent evt = {
        .event = event::Event{.description = Sdescription.eventListener, .entities = {1, 2}, .payload = 0.12}};

    Sdescription.system(Sdescription, combinations, evt);

    // Check that after execution, current active scene is Scene1
    REQUIRE(scenemgr.getCurrentSceneId() == 0);

    std::cout << "------Interpreter LoadScene------end" << std::endl;
}

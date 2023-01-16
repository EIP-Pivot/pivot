#include "pivot/ecs/Core/SceneManager.hxx"
#include "pivot/script/Engine.hxx"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

using namespace pivot::ecs;

TEST_CASE("Scripting-Interpreter-Entities")
{
    logger.start();

    logger.info() << "------Interpreter - Entities------start";

    component::Index cind;
    systems::Index sind;
    SceneManager m_scene_manager{};
    m_scene_manager.registerScene("Scene1");
    m_scene_manager.setCurrentSceneId(m_scene_manager.getSceneId("Scene1").value());
    script::Engine engine(
        sind, cind,
        pivot::ecs::script::interpreter::builtins::BuiltinContext{
            .isKeyPressed = [](auto) { return false; },
            .selectCamera = [](auto) { return false; },
            .createEntity = [&m_scene_manager](const std::string &name) -> std::pair<pivot::Entity, std::string> {
                //   EntityId =  currentScene->createEntity(name);
                pivot::Entity createdEntityId = 0;
                if (m_scene_manager.getCurrentScene().getEntityID(name).has_value()) {    // entity exists already
                    std::cout << "duplicating " << std::endl;
                    createdEntityId = m_scene_manager.getCurrentScene().CreateEntity(name + " - Copied");
                    return std::pair<pivot::Entity, std::string>(createdEntityId, name + " - Copied");
                } else {    // entity doesn't already exist
                    std::cout << "new one " << std::endl;
                    createdEntityId = m_scene_manager.getCurrentScene().CreateEntity(name);
                    return std::pair<pivot::Entity, std::string>(createdEntityId, name);
                }
            },
            .addComponent = [](Entity entityId, const std::string &entity, const std::string &component) -> void {
                // componentManager->getComponentId(component)
                // scene->AddComponent()
                return;
            }});
    // std::string file = "C:/Users/Najo/eip/pivot/libscript/tests/test_events.pivotscript";
    // engine.loadFile(file);

    std::string fileContent = "component Once\n"
                              "\tBoolean trigger\n"
                              "\n"
                              "system S(anyEntity<Once>) event Tick(Number deltaTime)\n"
                              "\temitEvent(\"Tick\")\n"
                              "\temitEvent(\"KeyPress\")\n";

    engine.loadFile(fileContent, true);

    REQUIRE(sind.getDescription("S").has_value());
    REQUIRE(cind.getDescription("Once").has_value());

    auto Oncedescription = cind.getDescription("Once").value();
    auto Sdescription = sind.getDescription("S").value();
    auto array1 = Oncedescription.createContainer(Oncedescription);
    std::vector<data::Value> entity = {data::Record{{"trigger", false}}};
    array1->setValueForEntity(0, entity.at(0));
    component::ArrayCombination combinations{{std::ref(*array1)}};
    event::EventWithComponent evt = {
        .event = event::Event{.description = Sdescription.eventListener, .entities = {1, 2}, .payload = 0.12}};

    std::vector<event::Event> triggeredEvents = Sdescription.system(Sdescription, combinations, evt);

    REQUIRE(triggeredEvents.size() == 2);
    REQUIRE(triggeredEvents.at(0).description.name == "Tick");
    REQUIRE(triggeredEvents.at(1).description.name == "KeyPress");

    logger.info() << "------Interpreter - Entities------end";
}

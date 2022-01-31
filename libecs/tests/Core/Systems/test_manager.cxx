#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"
#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Data/value.hxx>
#include <pivot/ecs/Core/Scene.hxx>
#include <pivot/ecs/Core/Systems/description.hxx>

using namespace pivot::ecs;
using namespace pivot::ecs::data;

void test_manager_registration(const systems::Description &description,
                               systems::Description::systemArgs &entities, const event::Event &event)
{
    std::cout << "I'm a tick system:\n";
    for (auto combination: entities) {
        auto tag = combination[1].get();
        std::cout << "\tEntity name = " << std::get<std::string>(std::get<Record>(tag).at("name")) << std::endl;
        std::get<std::string>(std::get<Record>(tag).at("name")) = "edit";
        combination[1].set(tag);
    }
    std::cout << "\tPayload type = " << event.payload.type() << std::endl;
    for (const auto &entity: event.entities)
        for (const auto &value: entity){
            std::cout << "\tEvent entity component = " << ((Value)value).type() << std::endl;
        }
}

TEST_CASE("Manager register system", "[description][registration][manager]")
{
    component::Manager cManager;
    EntityManager eManager;
    systems::Manager manager(cManager, eManager);

    component::Description tag = component::GlobalIndex::getSingleton().getDescription("Tag").value();
    component::Description rigid = component::GlobalIndex::getSingleton().getDescription("RigidBody").value();
    component::Description grav = component::GlobalIndex::getSingleton().getDescription("Gravity").value();

    component::Manager::ComponentId tagId = cManager.RegisterComponent(tag);
    component::Manager::ComponentId rigidId = cManager.RegisterComponent(rigid);
    component::Manager::ComponentId gravId = cManager.RegisterComponent(grav);

    // entity that match with system
    Entity entity = eManager.CreateEntity();
    cManager.AddComponent(entity, Value{Record{{"name", "entity 0"}}}, tagId);
    cManager.AddComponent(entity, Value{Record{{"velocity", glm::vec3(0.0f)}, {"acceleration", glm::vec3(0.0f)}}},
                           rigidId);
    cManager.AddComponent(entity, Value{Record{{"force", glm::vec3(0.0f)}}}, gravId);

    // entity that match with system
    entity = eManager.CreateEntity();
    cManager.AddComponent(entity, Value{Record{{"name", "entity 1"}}}, tagId);
    cManager.AddComponent(entity, Value{Record{{"velocity", glm::vec3(0.0f)}, {"acceleration", glm::vec3(0.0f)}}},
                           rigidId);
    cManager.AddComponent(entity, Value{Record{{"force", glm::vec3(0.0f)}}}, gravId);

    // entity not matching with system
    entity = eManager.CreateEntity();
    cManager.AddComponent(entity, Value{Record{{"velocity", glm::vec3(0.0f)}, {"acceleration", glm::vec3(0.0f)}}},
                           rigidId);
    cManager.AddComponent(entity, Value{Record{{"force", glm::vec3(0.0f)}}}, gravId);

    event::Description eventDescription{
        .name = "Tick",
        .entities = {
            {"Oui"}
        },
        .payload = BasicType::Number,
    };
    systems::Description description{
        .name = "Manager",
        .systemComponents =
            {
                "RigidBody",
                "Tag",
            },
        .eventListener = eventDescription,
        .eventComponents = {
            {"Tag"}
        },
        .system = &test_manager_registration,
    };
    systems::GlobalIndex::getSingleton().registerSystem(description);
    manager.useSystem(description);

    manager.execute(eventDescription, Value{1}, {0});
    manager.execute(eventDescription, Value{1}, {1});
}


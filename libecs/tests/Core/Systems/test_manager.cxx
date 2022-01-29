#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"
#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Scene.hxx>
#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/ecs/Core/Data/value.hxx>

using namespace pivot::ecs;
using namespace pivot::ecs::data;

void test_manager_registration(const systems::Description::availableEntities &e, const systems::Description &description,
                               const systems::Description::systemArgs &entities, const event::Event &event)
{
    std::cout << "I'm a tick system:\n";
    auto &tagArray = entities[1].get();
    for (const auto &entity: e) {
        auto tag = tagArray.getValueForEntity(entity).value();
        std::cout << std::get<std::string>(std::get<Record>(tag).at("name")) << std::endl;
        std::get<std::string>(std::get<Record>(tag).at("name")) = "edit";
        tagArray.setValueForEntity(entity, tag);
    }
}

TEST_CASE("Manager register system", "[description][registration][manager]")
{
    std::unique_ptr<component::Manager> cManager = std::make_unique<component::Manager>();
    std::unique_ptr<EntityManager> eManager = std::make_unique<EntityManager>();
    systems::Manager manager(cManager, eManager);

    component::Description tag =
        component::GlobalIndex::getSingleton().getDescription("Tag").value();
    component::Description rigid =
        component::GlobalIndex::getSingleton().getDescription("RigidBody").value();
    component::Description grav =
        component::GlobalIndex::getSingleton().getDescription("Gravity").value();

    component::Manager::ComponentId tagId = cManager->RegisterComponent(tag);
    component::Manager::ComponentId rigidId = cManager->RegisterComponent(rigid);
    component::Manager::ComponentId gravId = cManager->RegisterComponent(grav);

    // entity that match with system
    Entity entity = eManager->CreateEntity();
    cManager->AddComponent(entity, Value{Record{{"name", "entity 0"}}}, tagId);
    cManager->AddComponent(entity, Value{Record{{"velocity", glm::vec3(0.0f)}, {"acceleration", glm::vec3(0.0f)}}},
                          rigidId);
    cManager->AddComponent(entity, Value{Record{{"force", glm::vec3(0.0f)}}}, gravId);

    // entity that match with system
    entity = eManager->CreateEntity();
    cManager->AddComponent(entity, Value{Record{{"name", "entity 1"}}}, tagId);
    cManager->AddComponent(entity, Value{Record{{"velocity", glm::vec3(0.0f)}, {"acceleration", glm::vec3(0.0f)}}},
                           rigidId);
    cManager->AddComponent(entity, Value{Record{{"force", glm::vec3(0.0f)}}}, gravId);

    // entity not matching with system
    entity = eManager->CreateEntity();
    cManager->AddComponent(entity, Value{Record{{"velocity", glm::vec3(0.0f)}, {"acceleration", glm::vec3(0.0f)}}},
                           rigidId);
    cManager->AddComponent(entity, Value{Record{{"force", glm::vec3(0.0f)}}}, gravId);

    event::Description eventDescription {
        .name = "Tick",
        .entities = {},
        .payload = pivot::ecs::data::BasicType::Number,
    };
    systems::Description description{
        .name = "Manager",
        .components =
            {
                "RigidBody",
                "Tag",
            },
        .eventListener = eventDescription,
        .system = &test_manager_registration,
    };
    systems::GlobalIndex::getSingleton().registerSystem(description);
    manager.useSystem(description);

    event::Event event {
        .description = eventDescription,
        .entities = {},
        .payload = data::Value{1},
    };
    manager.execute(event);
    manager.execute(event);
}

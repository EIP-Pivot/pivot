#include "pivot/ecs/Core/EntityManager.hxx"

#include "pivot/ecs/Core/Event/description.hxx"
#include "pivot/ecs/Core/Event/index.hxx"
#include "pivot/ecs/Core/Event/manager.hxx"


#include "pivot/ecs/Core/Systems/description.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"


#include "pivot/ecs/Core/Data/value.hxx"
#include "pivot/ecs/Core/Scene.hxx"

#include <catch2/catch.hpp>

using namespace pivot::ecs;
using namespace pivot::ecs::data;

void tickSystem(const systems::Description &description, const systems::Description::systemArgs &entities,
                const event::Event &event)
{
    std::cout << "I'm a tick system:\n";
    auto &tagArray = entities[1].get();
    auto tag = tagArray.getValueForEntity(0).value();
    std::cout << "\tComponent Tag.name = " << std::get<std::string>(std::get<Record>(tag).at("name")) << std::endl;
    std::get<std::string>(std::get<Record>(tag).at("name")) = "edit";
    tagArray.setValueForEntity(0, tag);
}

TEST_CASE("Manager event", "[description][registration][manager]")
{
    std::unique_ptr<component::Manager> cManager = std::make_unique<component::Manager>();
    std::unique_ptr<EntityManager> eManager = std::make_unique<EntityManager>();
    std::unique_ptr<systems::Manager> sManager = std::make_unique<systems::Manager>(cManager, eManager);
    event::Manager eventManager(sManager);

    component::Description tag = component::GlobalIndex::getSingleton().getDescription("Tag").value();
    component::Description rigid = component::GlobalIndex::getSingleton().getDescription("RigidBody").value();
    component::Description grav = component::GlobalIndex::getSingleton().getDescription("Gravity").value();

    component::Manager::ComponentId tagId = cManager->RegisterComponent(tag);
    component::Manager::ComponentId rigidId = cManager->RegisterComponent(rigid);
    component::Manager::ComponentId gravId = cManager->RegisterComponent(grav);

    Entity entity = eManager->CreateEntity();
    cManager->AddComponent(entity, Value{Record{{"name", "oui"}}}, tagId);
    cManager->AddComponent(entity, Value{Record{{"velocity", glm::vec3(0.0f)}, {"acceleration", glm::vec3(0.0f)}}},
                           rigidId);
    cManager->AddComponent(entity, Value{Record{{"force", glm::vec3(0.0f)}}}, gravId);

    event::Description eventDescription {
        .name = "Tick",
        .entities = {},
        .payload = pivot::ecs::data::BasicType::Number,
    };

    event::GlobalIndex::getSingleton().registerEvent(eventDescription);

    systems::Description description{
        .name = "tickSystem",
        .systemComponents =
            {
                "RigidBody",
                "Tag",
            },
        .eventListener = eventDescription,
        .system = &tickSystem,
    };
    systems::GlobalIndex::getSingleton().registerSystem(description);
    sManager->useSystem(description);

    eventManager.sendEvent("Tick", Value{0.0f});
    eventManager.sendEvent("Tick", Value{0.0f});
}
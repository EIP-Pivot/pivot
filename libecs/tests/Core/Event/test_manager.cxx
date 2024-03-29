#include <catch2/catch_test_macros.hpp>

#include "pivot/ecs/Core/EntityManager.hxx"

#include "pivot/ecs/Core/Event/description.hxx"
#include "pivot/ecs/Core/Event/index.hxx"
#include "pivot/ecs/Core/Event/manager.hxx"

#include "pivot/ecs/Core/Systems/description.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"

#include "pivot/ecs/Core/Data/value.hxx"
#include "pivot/ecs/Core/Scene.hxx"
#include <pivot/ecs/Components/Tag.hxx>

#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>

using namespace pivot::ecs;
using namespace pivot::ecs::data;
using namespace pivot::builtins::components;

std::vector<event::Event> tickSystem(const systems::Description &, component::ArrayCombination &entities,
                                     const event::EventWithComponent &)
{
    for (auto combination: entities) {
        auto tag = combination[1].get();
        std::get<std::string>(std::get<Record>(tag).at("name")) = "edit";
        combination[1].set(tag);
    }
    return {};
}

TEST_CASE("Manager event", "[description][registration][manager]")
{
    component::Manager cManager;
    EntityManager eManager;
    systems::Manager sManager(cManager, eManager);
    event::Manager eventManager(sManager);

    component::Description tag = Tag::description;
    component::Description rigid = RigidBody::description;
    component::Description grav = Gravity::description;

    component::Manager::ComponentId tagId = cManager.RegisterComponent(tag);
    component::Manager::ComponentId rigidId = cManager.RegisterComponent(rigid);
    component::Manager::ComponentId gravId = cManager.RegisterComponent(grav);

    Entity entity = eManager.CreateEntity();
    cManager.AddComponent(entity, Value{Record{{"name", "oui"}}}, tagId);
    cManager.AddComponent(entity, Value{Record{{"velocity", glm::vec3(0.0f)}, {"acceleration", glm::vec3(0.0f)}}},
                          rigidId);
    cManager.AddComponent(entity, Value{Record{{"force", glm::vec3(0.0f)}}}, gravId);

    event::Description eventDescription{
        .name = "Tick",
        .entities = {},
        .payload = pivot::ecs::data::BasicType::Number,
    };

    // event::GlobalIndex::getSingleton().registerEvent(eventDescription);

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
    // systems::GlobalIndex::getSingleton().registerSystem(description);
    sManager.useSystem(description);

    auto tagValue = cManager.GetComponent(entity, tagId).value();
    auto name = std::get<std::string>(std::get<Record>(tagValue).at("name"));
    REQUIRE(name == "oui");
    event::Event event{
        .description = eventDescription,
        .payload = Value{0.0f},
    };
    eventManager.sendEvent(event);
    tagValue = cManager.GetComponent(entity, tagId).value();
    name = std::get<std::string>(std::get<Record>(tagValue).at("name"));
    REQUIRE(name == "edit");
}

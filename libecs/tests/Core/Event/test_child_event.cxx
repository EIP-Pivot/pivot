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

event::Description changeNameDescription{
    .name = "ChangeName",
    .entities = {"ToChange"},
    .payload = pivot::ecs::data::BasicType::String,
};

std::vector<event::Event> changeName(const systems::Description &, component::ArrayCombination &,
                                     const event::EventWithComponent &event)
{
    auto value = event.components[0][0];
    auto tag = value.get();
    std::get<std::string>(std::get<Record>(tag).at("name")) = std::get<std::string>(event.event.payload);
    value.set(tag);
    return {};
}

std::vector<event::Event> childTickSystem(const systems::Description &, component::ArrayCombination &entities,
                                          const event::EventWithComponent &)
{
    std::vector<event::Event> events;

    for (auto combination: entities) {
        event::Event childEvent{
            .description = changeNameDescription,
            .entities = {combination.entity},
            .payload = Value{"newName"},
        };
        events.push_back(childEvent);
    }
    return events;
}

TEST_CASE("Child event", "[event][child]")
{
    component::Manager cManager;
    EntityManager eManager;
    systems::Manager sManager(cManager, eManager);
    event::Manager eventManager(sManager);

    component::Description tag = Tag::description;

    component::Manager::ComponentId tagId = cManager.RegisterComponent(tag);

    Entity entity = eManager.CreateEntity();
    cManager.AddComponent(entity, Value{Record{{"name", "oui"}}}, tagId);

    event::Description tick{
        .name = "Tick",
        .entities = {},
        .payload = pivot::ecs::data::BasicType::Number,
    };

    systems::Description tickSystemDescription{
        .name = "tickSystemDescription",
        .systemComponents =
            {
                "Tag",
            },
        .eventListener = tick,
        .system = &childTickSystem,
    };

    systems::Description changeNameSystemDescription{
        .name = "changeNameSystem",
        .eventListener = changeNameDescription,
        .eventComponents = {{"Tag"}},
        .system = &changeName,
    };

    sManager.useSystem(tickSystemDescription);
    sManager.useSystem(changeNameSystemDescription);

    event::Event event{
        .description = tick,
        .payload = Value{0.0f},
    };
    auto tagValue = cManager.GetComponent(entity, tagId).value();
    auto name = std::get<std::string>(std::get<Record>(tagValue).at("name"));
    REQUIRE(name == "oui");

    eventManager.sendEvent(event);

    tagValue = cManager.GetComponent(entity, tagId).value();
    name = std::get<std::string>(std::get<Record>(tagValue).at("name"));
    REQUIRE(name == "newName");
}

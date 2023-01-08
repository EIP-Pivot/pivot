#pragma once

#include <pivot/builtins/events/collision.hxx>
#include <pivot/ecs/Core/Systems/description.hxx>

namespace pivot::builtins::systems
{
inline const pivot::ecs::systems::Description collisionTestSystem{
    .name = "Collision test",
    .entityName = "",
    .systemComponents = {"Transform"},
    .eventListener = events::collision,
    .eventComponents = {{}, {}},
    .provenance = ecs::Provenance::builtin(),
    .system = [](const pivot::ecs::systems::Description &, pivot::ecs::component::ArrayCombination &,
                 const pivot::ecs::event::EventWithComponent &) -> std::vector<pivot::ecs::event::Event> {
        logger.info() << "Collision test system called";
        return {};
    }};
}

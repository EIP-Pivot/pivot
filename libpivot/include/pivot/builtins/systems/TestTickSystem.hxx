#pragma once

#include <pivot/ecs/Core/Systems/description.hxx>

#include <pivot/builtins/events/tick.hxx>

namespace pivot::builtins::systems
{
inline const pivot::ecs::systems::Description testTickSystem{
    .name = "Test Tick",
    .entityName = "",
    .systemComponents = {},
    .eventListener = events::tick,
    .eventComponents = {},
    .provenance = pivot::ecs::Provenance::builtin(),
    .system = [](const auto &, pivot::ecs::component::ArrayCombination &combi,
                 const pivot::ecs::event::EventWithComponent &event) -> std::vector<pivot::ecs::event::Event> {
        assert(combi.arrays().size() == 0);
        logger.info() << "Called in a tick " << std::get<double>(event.event.payload);
        return {};
    },
};
}

#pragma once

#include <pivot/ecs/Core/Event/description.hxx>

namespace pivot::builtins::events
{
inline const pivot::ecs::event::Description editor_tick = {
    .name = "Editor tick",
    .entities = {},
    // FIXME: Put void
    .payload = pivot::ecs::data::BasicType::Number,
    .payloadName = "delta",
    .provenance = pivot::ecs::Provenance::builtin(),
};
}

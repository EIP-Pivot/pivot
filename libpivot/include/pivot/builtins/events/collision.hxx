#pragma once

#include <pivot/ecs/Core/Event/description.hxx>

namespace pivot::builtins::events
{
inline const pivot::ecs::event::Description collision = {
    .name = "Collision",
    .entities = {"collider1", "collider2"},
    .payload = pivot::ecs::data::BasicType::Void,
    .payloadName = "",
    .provenance = pivot::ecs::Provenance::builtin(),
};
}

#pragma once

#include <pivot/ecs/Core/Event/description.hxx>

namespace pivot::builtins::events
{
inline const pivot::ecs::event::Description keyPress = {
    .name = "KeyPress",
    .entities = {},
    .payload = pivot::ecs::data::BasicType::String,
    .payloadName = "key",
    .provenance = pivot::ecs::Provenance::builtin(),
};
}

#pragma once

#include <pivot/ecs/Core/Event/description.hxx>

namespace pivot::builtins::events
{
inline const pivot::ecs::event::Description tick = {
    .name = "Tick",
    .entities = {},
    .payload = pivot::ecs::data::BasicType::Number,
};
}

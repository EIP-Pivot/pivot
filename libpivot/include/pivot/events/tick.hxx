#pragma once

#include <pivot/ecs/Core/Event/description.hxx>

namespace pivot::events
{
inline const pivot::ecs::event::Description tick = {
    .name = "Tick",
    .entities = {},
    .payload = pivot::ecs::data::BasicType::Number,
};
}

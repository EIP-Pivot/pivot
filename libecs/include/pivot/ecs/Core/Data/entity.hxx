#pragma once

#include <pivot/ecs/Core/types.hxx>

namespace pivot::ecs::data
{
struct EntityRef {
    Entity ref;

    auto operator<=>(const EntityRef &) const = default;

    bool is_empty() const { return this->ref == NULL_ENTITY; }

    static constexpr EntityRef empty() { return EntityRef{NULL_ENTITY}; }
};
}

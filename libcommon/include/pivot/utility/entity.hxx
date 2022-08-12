#pragma once

#include <cinttypes>
#include <compare>
#include <limits>

namespace pivot
{
using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 500000;
const Entity NULL_ENTITY = std::numeric_limits<Entity>::max();

struct EntityRef {
    Entity ref;

    auto operator<=>(const EntityRef &) const = default;

    bool is_empty() const { return this->ref == NULL_ENTITY; }

    static constexpr EntityRef empty() { return EntityRef{NULL_ENTITY}; }
};

static_assert(sizeof(EntityRef) == sizeof(Entity), "Not transparent");
}    // namespace pivot

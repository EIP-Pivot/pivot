#pragma once

#include <cinttypes>
#include <compare>
#include <iostream>
#include <limits>

namespace pivot
{
using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 500000;
const Entity NULL_ENTITY = std::numeric_limits<Entity>::max();

struct EntityRef {
    Entity ref = NULL_ENTITY;

    auto operator<=>(const EntityRef &) const = default;

    bool is_empty() const { return this->ref == NULL_ENTITY; }

    static constexpr EntityRef empty() { return EntityRef{NULL_ENTITY}; }
};

static_assert(sizeof(EntityRef) == sizeof(Entity), "Not transparent");

inline std::ostream &operator<<(std::ostream &os, const EntityRef &entity)
{
    os << "Entity(";
    if (entity.is_empty()) {
        os << "NULL";
    } else {
        os << entity.ref;
    }
    return os << ")";
}
}    // namespace pivot

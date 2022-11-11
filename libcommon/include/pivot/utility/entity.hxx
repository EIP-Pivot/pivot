#pragma once

#include <cinttypes>
#include <compare>
#include <iostream>
#include <limits>

namespace pivot
{
/// @brief Id of an entity in the ECS
using Entity = std::uint32_t;
/// Maximum number of entity in the ECS
const Entity MAX_ENTITIES = 500000;
/// Special id of the null entity ref
const Entity NULL_ENTITY = std::numeric_limits<Entity>::max();

/// @brief Stored a reference to an entity in the ECS
struct EntityRef {
    /// Id of the entity
    Entity ref = NULL_ENTITY;

    /// Default comparison operator
    auto operator<=>(const EntityRef &) const = default;

    /// Is the reference empty ?
    bool is_empty() const { return this->ref == NULL_ENTITY; }

    /// Creates an empty EntityRef
    static constexpr EntityRef empty() { return EntityRef{NULL_ENTITY}; }
};

static_assert(sizeof(EntityRef) == sizeof(Entity), "Not transparent");

/// Print an entity reference
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

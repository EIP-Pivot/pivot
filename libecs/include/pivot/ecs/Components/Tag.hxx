#pragma once

#include <string>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::ecs
{
/// @struct Tag
///
/// @brief Tag component, used to name entities
struct Tag {
    /// Name of an entity
    std::string name;

    /// Default comparison
    auto operator<=>(const Tag &rhs) const = default;

    /// Component description
    static const pivot::ecs::component::Description description;
};

}    // namespace pivot::ecs

namespace std
{
template <>
/// implementation of the standards hash for the Tag
struct hash<pivot::ecs::Tag> {
    /// Call operator
    std::size_t operator()(const pivot::ecs::Tag &tagNative) const { return std::hash<std::string>()(tagNative.name); }
};

}    // namespace std
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

    /// Component description
    static const pivot::ecs::component::Description description;
};
}    // namespace pivot::ecs

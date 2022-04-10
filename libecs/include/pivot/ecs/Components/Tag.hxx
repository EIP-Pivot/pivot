#pragma once

#include <string>

#include <pivot/ecs/Core/Component/description.hxx>

/// @struct Tag
///
/// @brief Tag component, used to name entities
struct Tag {
    /// Name of an entity
    std::string name;

    /// Component description
    static const pivot::ecs::component::Description description;
};

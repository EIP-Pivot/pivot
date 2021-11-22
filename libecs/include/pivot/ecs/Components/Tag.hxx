#pragma once

#include <string>

#include <pivot/ecs/Core/Component/description.hxx>

/// @struct Tag
///
/// @brief Tag component, use to name entities
struct Tag {
    /// Name of an entity
    std::string name;

    static pivot::ecs::component::Description description;
};

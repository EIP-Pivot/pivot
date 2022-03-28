#pragma once

#include <glm/vec3.hpp>

#include <pivot/ecs/Core/Component/description.hxx>

/// @struct Gravity
///
/// @brief Gravity component (Example of gravity component)
struct Gravity {
    /// Gravity vector
    glm::vec3 force;

    /// Component description
    static const pivot::ecs::component::Description description;
};

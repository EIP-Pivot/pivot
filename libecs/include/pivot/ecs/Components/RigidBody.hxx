#pragma once

#include <glm/glm.hpp>

#include <pivot/ecs/Core/Component/description.hxx>

/// @struct RigidBody
///
/// @brief RigidBody component (Example of rigidbody component)
struct RigidBody {
    /// Velocity vector
    glm::vec3 velocity;
    /// Acceleration vector
    glm::vec3 acceleration;

    /// Component description
    static const pivot::ecs::component::Description description;
};

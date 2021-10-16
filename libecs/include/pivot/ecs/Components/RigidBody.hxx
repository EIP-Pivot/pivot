#pragma once

#include <glm/glm.hpp>

/// @struct RigidBody
///
/// @brief RigidBody component (Example of rigidbody component)
struct RigidBody {
    /// Velocity vector
    glm::vec3 velocity;
    /// Acceleration vector
    glm::vec3 acceleration;
};
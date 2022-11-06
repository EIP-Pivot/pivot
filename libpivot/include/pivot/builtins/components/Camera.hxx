#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::builtins::components
{
/// Component added to entity that generate collision events
struct Camera {
    /// Default camera yaw
    static constexpr const double YAW = -90.0;
    /// Default camera pitch
    static constexpr const double PITCH = 0.0;

    /// Direction vectors of the Camera
    struct Directions {
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
    };

    /// Focal point
    double fov;
    /// Camera
    double yaw = YAW;
    /// Camera pitch
    double pitch = PITCH;

    /// Default comparison operator
    auto operator<=>(const Camera &rhs) const = default;

    /// Component description
    static const pivot::ecs::component::Description description;
};
}    // namespace pivot::builtins::components

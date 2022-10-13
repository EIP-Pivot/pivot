#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::builtins::components
{
/// Component added to entity that generate collision events
struct Camera {
    /// Movements available on the camera
    enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };
    /// Maximum projection distance
    static constexpr const float MAX_PROJECTION_LIMIT = 10000.0f;
    /// Minimum projection distance
    static constexpr const float MIN_PROJECTION_LIMIT = 0.1f;
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

    /// Component description
    static const pivot::ecs::component::Description description;

    /// Get camera projection
    inline glm::mat4 getProjection(float fFOV, float fAspectRatio, float fCloseClippingPlane = MIN_PROJECTION_LIMIT,
                                   float fFarClippingPlane = MAX_PROJECTION_LIMIT) const
    {
        return glm::perspective(glm::radians(fFOV), fAspectRatio, fCloseClippingPlane, fFarClippingPlane);
    }

    inline Directions getDirections() const
    {
        const glm::vec3 WORLD_UP = {0.0f, 1.0f, 0.0f};

        glm::vec3 tmpFront;
        tmpFront.x = static_cast<float>(std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)));
        tmpFront.y = static_cast<float>(std::sin(glm::radians(pitch)));
        tmpFront.z = static_cast<float>(std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch)));

        glm::vec3 front = glm::normalize(tmpFront);
        glm::vec3 right = glm::normalize(glm::cross(tmpFront, WORLD_UP));
        glm::vec3 up = glm::normalize(glm::cross(right, tmpFront));

        return Directions{front, right, up};
    }

    /// Get camera view
    inline glm::mat4 getView(glm::vec3 position) const
    {
        auto directions = this->getDirections();
        return glm::lookAt(position, position + directions.front, directions.up);
    }
};
}    // namespace pivot::builtins::components

#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <pivot/builtins/components/Camera.hxx>
#include <pivot/graphics/types/Transform.hxx>
#include <pivot/graphics/types/vk_types.hxx>

namespace pivot::internals
{
/// Component added to entity that generate collision events
struct LocationCamera {
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

    /// Camera part
    builtins::components::Camera camera;
    /// Transform part
    graphics::Transform transform;

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
        tmpFront.x = static_cast<float>(std::cos(glm::radians(camera.yaw)) * std::cos(glm::radians(camera.pitch)));
        tmpFront.y = static_cast<float>(std::sin(glm::radians(camera.pitch)));
        tmpFront.z = static_cast<float>(std::sin(glm::radians(camera.yaw)) * std::cos(glm::radians(camera.pitch)));

        glm::vec3 front = glm::normalize(tmpFront);
        glm::vec3 right = glm::normalize(glm::cross(tmpFront, WORLD_UP));
        glm::vec3 up = glm::normalize(glm::cross(right, tmpFront));

        return Directions{front, right, up};
    }

    /// Get camera view
    inline glm::mat4 getView() const
    {
        auto directions = this->getDirections();
        return glm::lookAt(this->transform.position, this->transform.position + directions.front, directions.up);
    }

    /// Get CameraData of the camera
    graphics::CameraData
    getGPUCameraData(float fFOV, float fAspectRatio,
                     float fCloseClippingPlane = builtins::components::Camera::MIN_PROJECTION_LIMIT,
                     float fFarClippingPlane = builtins::components::Camera::MAX_PROJECTION_LIMIT);
};
}    // namespace pivot::internals

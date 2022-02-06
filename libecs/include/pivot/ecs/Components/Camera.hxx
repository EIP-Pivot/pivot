#pragma once

#include <pivot/graphics/types/vk_types.hxx>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#ifndef PIVOT_MAX_PROJECTION_LIMIT
#define PIVOT_MAX_PROJECTION_LIMIT 10000.0f
#endif

#ifndef PIVOT_MIN_PROJECTION_LIMIT
#define PIVOT_MIN_PROJECTION_LIMIT 0.1f
#endif

/// @class Camera
///
/// @brief Camera component (Component exemple)
class Camera
{
public:
    /// @cond
    enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };
    static constexpr const double YAW = -90.0;
    static constexpr const double PITCH = 0.0;
    /// @endcond

public:
    /// Default Constructor, can be init with default position
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
        : position(position), up(up)
    {
    }

    /// @cond
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    double yaw = YAW;
    double pitch = PITCH;
    /// @endcond

    /// Get camera projection
    inline glm::mat4 getProjection(float fFOV, float fAspectRatio,
                                   float fCloseClippingPlane = PIVOT_MIN_PROJECTION_LIMIT,
                                   float fFarClippingPlane = PIVOT_MAX_PROJECTION_LIMIT) const
    {
        return glm::perspective(glm::radians(fFOV), fAspectRatio, fCloseClippingPlane, fFarClippingPlane);
    }

    /// Get camera view
    inline glm::mat4 getView() const { return glm::lookAt(position, position + front, up); }

    /// Get CameraData of the camera
    pivot::graphics::CameraData getGPUCameraData(float fFOV, float fAspectRatio,
                                                 float fCloseClippingPlane = PIVOT_MIN_PROJECTION_LIMIT,
                                                 float fFarClippingPlane = PIVOT_MAX_PROJECTION_LIMIT) const
    {
        auto projection = getProjection(fFOV, fAspectRatio, fCloseClippingPlane, fFarClippingPlane);
        projection[1][1] *= -1;
        auto view = getView();
        return {
            .position = position,
            .view = view,
            .projection = projection,
            .viewProjection = projection * view,
        };
    }

    /// @cond
    void updateCameraVectors()
    {
        glm::vec3 tmpFront;
        tmpFront.x = static_cast<float>(std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)));
        tmpFront.y = static_cast<float>(std::sin(glm::radians(pitch)));
        tmpFront.z = static_cast<float>(std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch)));

        front = glm::normalize(tmpFront);
        right = glm::normalize(glm::cross(tmpFront, worldUp));
        up = glm::normalize(glm::cross(right, tmpFront));
    }
    /// @endcond
};
#pragma once

#include "pivot/graphics/interface/ICamera.hxx"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Camera : public ICamera
{
public:
    /// @cond
    enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };
    static constexpr const double YAW = -90.0;
    static constexpr const double PITCH = 0.0;
    /// @endcond

public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
        : position(position), up(up)
    {
    }

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    double yaw = YAW;
    double pitch = PITCH;

    glm::mat4 getProjection(float fFOV = 70.f, float fAspectRatio = 1700.f / 900.f, float fCloseClippingPlane = 0.1,
                            float fFarClippingPlane = MAX_PROJECTION_LIMIT) const
    {
        return glm::perspective(glm::radians(fFOV), fAspectRatio, fCloseClippingPlane, fFarClippingPlane);
    }

    glm::mat4 getView() const
    {
        return glm::lookAt(position, position + front, up);
    }

    GPUCameraData getGPUCameraData(float fFOV = 70.f, float fAspectRatio = 1700.f / 900.f,
                                   float fCloseClippingPlane = 0.1,
                                   float fFarClippingPlane = MAX_PROJECTION_LIMIT) const final
    {
        auto projection = getProjection(fFOV, fAspectRatio, fCloseClippingPlane, fFarClippingPlane);
        projection[1][1] *= -1;
        auto view = getView();
        GPUCameraData data{
            .position = glm::vec4(position, 1.0f),
            .viewproj = projection * view,
        };
        return data;
    }

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
};
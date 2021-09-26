#pragma once

#include "pivot/graphics/interface/ICamera.hxx"

#include <glm/vec3.hpp>

/// @class Camera
///
/// @brief Represent a Camera in 3D space
class Camera : public ICamera
{
public:
    /// @cond
    enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };
    static constexpr const float YAW = -90.0f;
    static constexpr const float PITCH = 0.0f;
    static constexpr const float SPEED = 2.5f;
    static constexpr const float JUMP = 2.5f;
    static constexpr const float SENSITIVITY = 0.1f;
    /// @endcond

public:
    /// Constructor
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = Camera::YAW, float pitch = Camera::PITCH);

    /// Return the view matrix of the camera
    glm::mat4 getViewMatrix() const noexcept;

    /// Return the expected data required by the shaders
    GPUCameraData getGPUCameraData(float fFOV = 70.f, float fAspectRatio = 1700.f / 900.f,
                                   float fCloseClippingPlane = 0.1,
                                   float fFarClippingPlane = MAX_PROJECTION_LIMIT) const final;

    /// @cond
    void processKeyboard(Movement direction) noexcept;
    void processMouseMovement(float xoffset, float yoffset, bool bConstrainPitch = true);

protected:
    void updateCameraVectors();

public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float movementSpeed = SPEED;
    float jumpHeight = JUMP;
    float mouseSensitivity = SENSITIVITY;
    /// @endcond
};

#pragma once

#include <glm/glm.hpp>

#ifndef MAX_PROJECTION_LIMIT
#define MAX_PROJECTION_LIMIT 100.0f
#endif

class Camera
{
public:
    struct GPUCameraData {
        glm::vec4 position;
        glm::mat4 viewproj;
    };
    enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };
    static constexpr const float YAW = -90.0f;
    static constexpr const float PITCH = 0.0f;
    static constexpr const float SPEED = 25.0f;
    static constexpr const float JUMP = 50.0f;

    static constexpr const float SENSITIVITY = 0.1f;

public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = Camera::YAW, float pitch = Camera::PITCH);
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
    glm::mat4 getViewMatrix() const;
    GPUCameraData getGPUCameraData(float fFOV = 70.f, float fAspectRatio = 1700.f / 900.f,
                                   float fCloseClippingPlane = 0.1,
                                   float fFarClippingPlane = MAX_PROJECTION_LIMIT) const;
    void processKeyboard(Movement direction);
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
};

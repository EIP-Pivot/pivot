#pragma once

#include "pivot/ecs/Components/Camera.hxx"
#include "pivot/ecs/Core/Event.hxx"
#include "pivot/ecs/Core/System.hxx"
#include "pivot/graphics/Window.hxx"

#include <optional>

class ControlSystem : public System
{
public:
    static constexpr const float SPEED = 2.5f;
    static constexpr const float JUMP = 2.5f;
    static constexpr const float SENSITIVITY = 0.5f;
    static constexpr const bool CONSTRAIN_PITCH = true;

public:
    void Init();

    virtual void Update(float dt);

    static void processMouseMovement(Camera &cam, const glm::dvec2 &offset);
private:
    std::bitset<UINT16_MAX> button;
    std::optional<glm::dvec2> offset;

    void InputListener(Event &event);
    void MouseListener(Event &event);

    static void processKeyboard(Camera &cam, const Camera::Movement direction, float dt) noexcept;
};
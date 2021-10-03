#pragma once

#include "pivot/ecs/Components/Camera.hxx"
#include "pivot/ecs/Core/Event.hxx"
#include "pivot/ecs/Core/System.hxx"
#include "pivot/graphics/Window.hxx"

#include <optional>

class ControlSystem : public System
{
public:
    static constexpr const double SPEED = 2.5;
    static constexpr const double JUMP = 2.5;
    static constexpr const double SENSITIVITY = 0.5;
    static constexpr const bool CONSTRAIN_PITCH = true;

public:
    void Init();

    void Update(float dt);

private:
    std::bitset<UINT16_MAX> button;
    std::optional<glm::dvec2> offset;

    void InputListener(Event &event);
    void MouseListener(Event &event);

    static void processKeyboard(Camera &cam, const Camera::Movement direction) noexcept;
    static void processMouseMovement(Camera &cam, const glm::dvec2 &offset);
};
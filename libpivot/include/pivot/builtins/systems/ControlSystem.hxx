#pragma once

#include "pivot/ecs/Components/Camera.hxx"
#include "pivot/ecs/Core/Event.hxx"
#include "pivot/ecs/Core/System.hxx"
#include "pivot/graphics/Window.hxx"

#include <optional>

namespace pivot::builtins::systems
{
/// @class ControlSystem
///
/// @brief Control system example, to control camera movement
class ControlSystem : public System
{
public:
    /// Speed movement
    static constexpr const float SPEED = 2.5f;
    /// Jump height
    static constexpr const float JUMP = 2.5f;
    /// Sensitivity of the mouse
    static constexpr const float SENSITIVITY = 0.5f;
    /// @cond
    static constexpr const bool CONSTRAIN_PITCH = true;
    /// @endcond

public:
    /// Init system
    // void Init();

    /// Update camera movement when key press
    // virtual void Update(float dt);

    /// Update mouse movement (Called in Update())
    static void processMouseMovement(Camera &cam, const glm::dvec2 &offset);

private:
    // std::bitset<UINT16_MAX> button;
    // std::optional<glm::dvec2> offset;

    // void InputListener(Event &event);
    // void MouseListener(Event &event);

    // static void processKeyboard(Camera &cam, const Camera::Movement direction, float dt) noexcept;
};
}    // namespace pivot::builtins::systems

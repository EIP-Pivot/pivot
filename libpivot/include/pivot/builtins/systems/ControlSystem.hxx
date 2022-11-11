#pragma once

#include <optional>

#include <pivot/builtins/components/Camera.hxx>
#include <pivot/ecs/Core/Event/description.hxx>
#include <pivot/graphics/Window.hxx>

namespace pivot::builtins::systems
{
/// @class ControlSystem
///
/// @brief Control system example, to control camera movement
class ControlSystem
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
    static std::vector<pivot::ecs::event::Event> processMouseMovement(components::Camera &cam,
                                                                      const glm::dvec2 &offset);

private:
    // std::bitset<UINT16_MAX> button;
    // std::optional<glm::dvec2> offset;

    // void InputListener(Event &event);
    // void MouseListener(Event &event);

    // static void processKeyboard(Camera &cam, const Camera::Movement direction, float dt) noexcept;
};
}    // namespace pivot::builtins::systems

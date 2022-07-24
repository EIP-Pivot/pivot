#pragma once

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::builtins::components
{
struct Transform2D {
    /// Position on the screen
    glm::vec2 position;

    /// Rotation on the screen
    double rotation;

    /// Scale on the screen
    glm::vec2 scale;

    /// Component description
    static const pivot::ecs::component::Description description;
};
}    // namespace pivot::builtins::components

#pragma once

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::builtins::components
{
/// Component added to entity that generate collision events
struct Camera {
    /// Focal point
    double fov;
    /// Component description
    static const pivot::ecs::component::Description description;
};
}    // namespace pivot::builtins::components

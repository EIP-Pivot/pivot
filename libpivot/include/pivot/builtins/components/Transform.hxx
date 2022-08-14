#pragma once

#include "pivot/graphics/types/Transform.hxx"
#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::builtins::components
{
struct Transform : public pivot::graphics::Transform {
    /// Component description
    static const pivot::ecs::component::Description description;
};
}    // namespace pivot::builtins::components

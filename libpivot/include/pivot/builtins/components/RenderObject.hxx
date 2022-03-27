#pragma once

#include "pivot/graphics/types/RenderObject.hxx"
#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::builtins::components
{
struct RenderObject : public pivot::graphics::RenderObject {
    /// Component description
    static const pivot::ecs::component::Description description;
};
}    // namespace pivot::builtins::components

#pragma once

#include "pivot/graphics/types/RenderObject.hxx"
#include <pivot/ecs/Core/Component/description.hxx>

struct RenderObject : public pivot::graphics::RenderObject {
    /// Component description
    static const pivot::ecs::component::Description description;
};

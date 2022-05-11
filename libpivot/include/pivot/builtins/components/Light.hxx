#pragma once

#include "pivot/graphics/types/Light.hxx"
#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::builtins::components
{
struct DirectionalLight : public pivot::graphics::DirectionalLight {
    /// Component description
    static const pivot::ecs::component::Description description;
};

struct PointLight : public pivot::graphics::PointLight {
    /// Component description
    static const pivot::ecs::component::Description description;
};

struct SpotLight : public pivot::graphics::SpotLight {
    /// Component description
    static const pivot::ecs::component::Description description;
};

}    // namespace pivot::builtins::components

#pragma once

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::builtins::components
{
struct Text {
    /// Content of the text
    std::string content;

    /// Color of the text
    pivot::ecs::data::Color color;

    /// Component description
    static const pivot::ecs::component::Description description;
};
}    // namespace pivot::builtins::components

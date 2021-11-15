#pragma once

#include "pivot/ecs/Core/Component/description.hxx"

namespace pivot::ecs::persistence
{
std::string serializeComponent(const pivot::ecs::component::Description &description, const std::any &component);
}
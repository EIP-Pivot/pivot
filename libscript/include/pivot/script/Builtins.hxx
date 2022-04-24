#pragma once

#include "pivot/ecs/Core/Data/value.hxx"

#include <vector>

namespace pivot::ecs::script::interpreter::builtins {

/// Boolean	isPressed(String key)
data::Value builtin_isPressed(const std::vector<data::Value> &params);
data::Value builtin_print(const std::vector<data::Value> &params);


} // end of namespace pivot::ecs::script::interpreter::builtins

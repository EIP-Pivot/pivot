#pragma once

#include "pivot/ecs/Core/Data/value.hxx"

#include <vector>

namespace pivot::ecs::script::interpreter::builtins
{

/// Builtins assume that the interpreter has called interpreter::validateParams()
/// therefore that the parameters are of correct size and types

/// Boolean	isPressed(String key)
///		Returns whether or not the keyboard key is pressed
data::Value builtin_isPressed(const std::vector<data::Value> &params);

// void	print(String/Number/Boolean param1, ...)
//		print the parameters on a single line, ending with a newline
data::Value builtin_print(const std::vector<data::Value> &params);

// Operators

enum class Operator {
    Equal,
    NotEqual,
    GreaterThan,
    GreaterOrEqual,
    LowerThan,
    LowerOrEqual,
    Addition,
    Substraction,
    Multiplication,
    Divison,
    Modulo,
    LogicalAnd,
    LogicalOr,
};

template <Operator O>
data::Value builtin_operator(const data::Value &left, const data::Value &right);

}    // end of namespace pivot::ecs::script::interpreter::builtins

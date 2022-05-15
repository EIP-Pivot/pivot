#pragma once

#include <functional>
#include <iostream>
#include <vector>

#include "pivot/ecs/Core/Data/value.hxx"

namespace pivot::ecs::script::interpreter::builtins
{

/// Builtins assume that the interpreter has called interpreter::validateParams()
/// therefore that the parameters are of correct size and types

/// Context given to the builtins to interact with the outside
struct BuiltinContext {
    /// Functor returning true if a specific key is pressed
    std::function<bool(const std::string &)> isKeyPressed;
};

/// Boolean	isPressed(String key)
///		Returns whether or not the keyboard key is pressed
data::Value builtin_isPressed(const std::vector<data::Value> &params, const BuiltinContext &context);

// void	print(String/Number/Boolean param1, ...)
//		print the parameters on a single line, ending with a newline
data::Value builtin_print(const std::vector<data::Value> &params, const BuiltinContext &context);

/// Same as above but take a ostream to write to
data::Value builtin_print_stream(const std::vector<data::Value> &params, std::ostream &stream);

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

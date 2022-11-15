#pragma once

#include <cmath>
#include <functional>
#include <iostream>
#include <vector>

#include "pivot/ecs/Core/Data/value.hxx"
#include "pivot/script/Exceptions.hxx"

namespace pivot::ecs::script::interpreter::builtins
{

/// Builtins assume that the interpreter has called interpreter::validateParams()
/// therefore that the parameters are of correct size and types

/// Context given to the builtins to interact with the outside
struct BuiltinContext {
    /// Functor returning true if a specific key is pressed
    std::function<bool(const std::string &)> isKeyPressed;

    /// Functor calling the setScene from ecs::ScenemManager
    std::function<void(const std::string &)> loadScene;

    /// Select an entity as the new current camera
    std::function<void(std::optional<Entity>)> selectCamera;

    /// Mock builtin context for unit testing
    static BuiltinContext mock()
    {
        return BuiltinContext{.isKeyPressed = [](auto) { return false; }, .loadScene = [](auto) {}, .selectCamera = [](auto) {}};
    }
};

/// Boolean	isPressed(String key)
///		Returns whether or not the keyboard key is pressed
data::Value builtin_isPressed(const std::vector<data::Value> &params, const BuiltinContext &context);

// void	print(String/Number/Boolean param1, ...)
//		print the parameters on a single line, ending with a newline
data::Value builtin_print(const std::vector<data::Value> &params, const BuiltinContext &context);

/// Same as above but take a ostream to write to
data::Value builtin_print_stream(const std::vector<data::Value> &params, std::ostream &stream);

/// void	loadScene(String scene)
///		Loads the scene string passed as parameter into the editor
data::Value builtin_loadScene(const std::vector<data::Value> &params, const BuiltinContext &context);

// Math built-ins
/// Number cos(Number radAngle)
///		Returns the cosine of the parameter (in radian)
data::Value builtin_cos(const std::vector<data::Value> &params, const BuiltinContext &context);

// Math built-ins
/// Number sin(Number radAngle)
///		Returns the sine of the parameter (in radian)
data::Value builtin_sin(const std::vector<data::Value> &params, const BuiltinContext &context);

/// Number  randint(max x)
///     Returns a pseudo-random integral value in [0, x]
data::Value builtin_randint(const std::vector<data::Value> &params, const BuiltinContext &context);

/// Number  pow(Number x, Number y)
///     Returns the power of a number.
data::Value builtin_power(const std::vector<data::Value> &params, const BuiltinContext &context);

/// Number  sqrt(Number x)
///     Returns the square root of the given Number
data::Value builtin_sqrt(const std::vector<data::Value> &params, const BuiltinContext &context);

/// Number  abs(Number x)
///     Returns the absolute value of the given Number
data::Value builtin_abs(const std::vector<data::Value> &params, const BuiltinContext &context);

/// String	toString(Any val)
///		Returns a string representation of the given Value
data::Value builtin_toString(const std::vector<data::Value> &params, const BuiltinContext &);

/// Vec3  vec3(Number x, Number y, Number z)
///     Returns a built Vector3 value from the xyz parameters
data::Value builtin_vec3(const std::vector<data::Value> &params, const BuiltinContext &context);

/// Color  color(Number r, Number g, Number b, Number a)
///     Returns a built Color value from the rgba parameters
data::Value builtin_color(const std::vector<data::Value> &params, const BuiltinContext &context);

/// Void selectCamera(Entity entity)
data::Value builtin_selectCamera(const std::vector<data::Value> &params, const BuiltinContext &context);

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

constexpr auto generic_builtin_comparator(auto op, const std::string &op_string)
{
    return [op, op_string](const data::Value &left_value, const data::Value &right_value) {
        return std::visit(
            [&left_value, &right_value, &op_string, &op](const auto &left, const auto &right) -> bool {
                using L = std::decay_t<decltype(left)>;
                using R = std::decay_t<decltype(right)>;
                if constexpr (std::predicate<decltype(op), L, R>) { return op(left, right); }
                logger.err("ERROR") << " by '" << left_value.type().toString() << "' and '"
                                    << right_value.type().toString() << "'";
                throw InvalidOperation(std::string("Invalid equal to '") + op_string +
                                       "'operator between these types.");
            },
            static_cast<const data::Value::variant &>(left_value),
            static_cast<const data::Value::variant &>(right_value));
    };
}

}    // end of namespace pivot::ecs::script::interpreter::builtins

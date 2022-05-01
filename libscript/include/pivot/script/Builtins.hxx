#pragma once

#include "pivot/ecs/Core/Data/value.hxx"

#include <vector>

namespace pivot::ecs::script::interpreter::builtins
{

/// Boolean	isPressed(String key)
data::Value builtin_isPressed(const std::vector<data::Value> &params);
data::Value builtin_print(const std::vector<data::Value> &params);

// Operators

// Relational operators
data::Value builtin_operatorEq(const data::Value &left, const data::Value &right);     // equal to operator '=='
data::Value builtin_operatorNEq(const data::Value &left, const data::Value &right);    // not equal to operator '!='
data::Value builtin_operatorGt(const data::Value &left, const data::Value &right);     // greater than operator
data::Value builtin_operatorGtEq(const data::Value &left,
                                 const data::Value &right);    // greater than or equal to operator '>='
data::Value builtin_operatorLt(const data::Value &left, const data::Value &right);    // lower than operator '<'
data::Value builtin_operatorLtEq(const data::Value &left,
                                 const data::Value &right);    // lower than or equal to operator '<='

// Mathematical/Arithmetic operators
data::Value builtin_operatorAdd(const data::Value &left, const data::Value &right);    // additive operator '+'
data::Value builtin_operatorSub(const data::Value &left, const data::Value &right);    // additive operator '-'
data::Value builtin_operatorMul(const data::Value &left, const data::Value &right);    // multiplication operator '*'
data::Value builtin_operatorDiv(const data::Value &left, const data::Value &right);    // division operator '/'
data::Value builtin_operatorMod(const data::Value &left, const data::Value &right);    // modulo operator '%'

// Logical operators
data::Value builtin_operatorAnd(const data::Value &left, const data::Value &right);    // logical operator '&&' AND
data::Value builtin_operatorOr(const data::Value &left, const data::Value &right);     // logical operator '||' OR

}    // end of namespace pivot::ecs::script::interpreter::builtins

#include "pivot/script/Exceptions.hxx"
#include "pivot/script/Builtins.hxx"
#include "Logger.hpp"

#include <iostream>

namespace pivot::ecs::script::interpreter::builtins {

// Builtins assume that the interpreter has called interpreter::validateParams()
// therefore that the parameters are of correct size and types

// Boolean	isPressed(String key)
//		Returns whether or not the keyboard key is pressed
data::Value builtin_isPressed(const std::vector<data::Value> &params) {
	// TODO : find a way to pass Window * as parameter
	return data::Value(true);
}

// void	print(String/Number/Boolean param1, ...)
//		print the parameters on a single line, ending with a newline
data::Value builtin_print(const std::vector<data::Value> &params) {
	for (const data::Value &param : params) { // print has unlimited number of parameters
		data::BasicType varType = std::get<data::BasicType>(param.type());
		if (varType == data::BasicType::Number)
			std::cout << std::get<double>(param) << " ";
		else if (varType == data::BasicType::String)
			std::cout << std::get<std::string>(param) << " ";
		else if (varType == data::BasicType::Boolean)
			std::cout << (std::get<bool>(param) ? "true" : "false") << " ";
	}
	std::cout << "\n";
	return data::Value();
}


// Builtin binary (which take two operands) operators
// TOOD: find a better (template?) solution to handle these cases

// Relational operators -- start

data::Value builtin_operatorEq(const data::Value &left, const data::Value &right) { // equal to operator '=='
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Number && std::get<data::BasicType>(right.type()) == data::BasicType::Number)
		return data::Value(std::get<double>(left) == std::get<double>(right)); // perform arithmetic comparison
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::String && std::get<data::BasicType>(right.type()) == data::BasicType::String)
		return data::Value(std::get<std::string>(left) == std::get<std::string>(right)); // perform alphabetical comparison
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::Boolean && std::get<data::BasicType>(right.type()) == data::BasicType::Boolean)
		return data::Value(std::get<bool>(left) == std::get<bool>(right)); // perform arithmetic comparison
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 && std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
		return data::Value(std::get<glm::vec3>(left) == std::get<glm::vec3>(right)); // let glm do the comparison
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid equal to '==' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid equal to '==' operator between these types.");
}}
data::Value builtin_operatorNEq(const data::Value &left, const data::Value &right) { // not equal to operator '!='
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Number && std::get<data::BasicType>(right.type()) == data::BasicType::Number)
		return data::Value(std::get<double>(left) != std::get<double>(right)); // perform arithmetic comparison
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::String && std::get<data::BasicType>(right.type()) == data::BasicType::String)
		return data::Value(std::get<std::string>(left) != std::get<std::string>(right)); // perform alphabetical comparison
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::Boolean && std::get<data::BasicType>(right.type()) == data::BasicType::Boolean)
		return data::Value(std::get<bool>(left) != std::get<bool>(right)); // perform arithmetic comparison
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 && std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
		return data::Value(std::get<glm::vec3>(left) != std::get<glm::vec3>(right)); // let glm do the comparison
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid not equal to '!=' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid not equal to '!=' operator between these types.");
}}
data::Value builtin_operatorGt(const data::Value &left, const data::Value &right) { // greater than operator '>'
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Number && std::get<data::BasicType>(right.type()) == data::BasicType::Number)
		return data::Value(std::get<double>(left) > std::get<double>(right)); // perform arithmetic comparison
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::String && std::get<data::BasicType>(right.type()) == data::BasicType::String)
		return data::Value(std::get<std::string>(left) > std::get<std::string>(right)); // perform alphabetical comparison
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid greater than '>' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid greater than '>' operator between these types.");
}}
data::Value builtin_operatorGtEq(const data::Value &left, const data::Value &right) { // greater than or equal to operator '>='
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Number && std::get<data::BasicType>(right.type()) == data::BasicType::Number)
		return data::Value(std::get<double>(left) >= std::get<double>(right)); // perform arithmetic comparison
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::String && std::get<data::BasicType>(right.type()) == data::BasicType::String)
		return data::Value(std::get<std::string>(left) >= std::get<std::string>(right)); // perform alphabetical comparison
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid greater than or equal to '>=' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid greater than or equal to '>=' operator between these types.");
}}
data::Value builtin_operatorLt(const data::Value &left, const data::Value &right) { // lower than operator '<'
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Number && std::get<data::BasicType>(right.type()) == data::BasicType::Number)
		return data::Value(std::get<double>(left) < std::get<double>(right)); // perform arithmetic comparison
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::String && std::get<data::BasicType>(right.type()) == data::BasicType::String)
		return data::Value(std::get<std::string>(left) < std::get<std::string>(right)); // perform alphabetical comparison
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid lower than '<' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid lower than '<' operator between these types.");
}}
data::Value builtin_operatorLtEq(const data::Value &left, const data::Value &right) { // lower than or equal to operator '<='
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Number && std::get<data::BasicType>(right.type()) == data::BasicType::Number)
		return data::Value(std::get<double>(left) <= std::get<double>(right)); // perform arithmetic comparison
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::String && std::get<data::BasicType>(right.type()) == data::BasicType::String)
		return data::Value(std::get<std::string>(left) <= std::get<std::string>(right)); // perform alphabetical comparison
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid lower than or equal to '<=' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid lower than or equal to '<=' operator between these types.");
}}

// Relational operators -- end

// Mathematical/Arithmetic operators -- start
data::Value builtin_operatorAdd(const data::Value &left, const data::Value &right) { // additive operator '+'
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Number && std::get<data::BasicType>(right.type()) == data::BasicType::Number)
		return data::Value(std::get<double>(left) + std::get<double>(right)); // perform arithmetic addition
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::String && std::get<data::BasicType>(right.type()) == data::BasicType::String)
		return data::Value(std::get<std::string>(left) + std::get<std::string>(right)); // perform string concatenation
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 && std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
		return data::Value(std::get<glm::vec3>(left) + std::get<glm::vec3>(right)); // let glm add the vectors
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid addition '+' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid addition '+' operator between these types.");
}}
data::Value builtin_operatorSub(const data::Value &left, const data::Value &right) { // additive operator '-'
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Number && std::get<data::BasicType>(right.type()) == data::BasicType::Number)
		return data::Value(std::get<double>(left) - std::get<double>(right)); // perform arithmetic substraction
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 && std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
		return data::Value(std::get<glm::vec3>(left) - std::get<glm::vec3>(right)); // let glm substract the vectors
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid substraction '-' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid substraction '-' operator between these types.");
}}
data::Value builtin_operatorMul(const data::Value &left, const data::Value &right) { // multiplication operator '*'
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Number && std::get<data::BasicType>(right.type()) == data::BasicType::Number)
		return data::Value(std::get<double>(left) * std::get<double>(right)); // perform arithmetic multiplication
	else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 && std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
		return data::Value(std::get<glm::vec3>(left) * std::get<glm::vec3>(right)); // let glm multiply the vectors
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid multiplication '*' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid multiplication '*' operator between these types.");
}}
data::Value builtin_operatorDiv(const data::Value &left, const data::Value &right) { // division operator '/'
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Number && std::get<data::BasicType>(right.type()) == data::BasicType::Number) {
		if (std::get<double>(right) == 0.0) // handle div by zero
			throw InvalidException("DivByZero", std::to_string(std::get<double>(left)) + " / 0", "Cannot divide by zero.");
		return data::Value(std::get<double>(left) / std::get<double>(right)); // perform arithmetic division
	} else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 && std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
		return data::Value(std::get<glm::vec3>(left) / std::get<glm::vec3>(right)); // let glm divide the vectors TODO: check it throws on div by zero
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid division '-' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid division '-' operator between these types.");
}}
data::Value builtin_operatorMod(const data::Value &left, const data::Value &right) { // modulo operator '%'
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Integer && std::get<data::BasicType>(right.type()) == data::BasicType::Integer) {
		if (std::get<int>(right) == 0) // handle modulo by zero
			throw InvalidException("DivByZero", std::to_string(std::get<int>(left)) + " / 0", "Cannot modulo by zero.");
		return data::Value(std::get<int>(left) % std::get<int>(right)); // perform arithmetic modulo
	} else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid modulo '%' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid modulo '%' operator between these types.");
}}

// Mathematical/Arithmetic operators -- end

// Logical operators -- start

data::Value builtin_operatorAnd(const data::Value &left, const data::Value &right) { // logical operator '&&' AND
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Boolean && std::get<data::BasicType>(right.type()) == data::BasicType::Boolean)
		return data::Value(std::get<bool>(left) && std::get<bool>(right)); // perform logical AND
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid Logical AND '&&' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid Logical AND '&&' operator between these types.");
}}
data::Value builtin_operatorOr(const data::Value &left, const data::Value &right) { // logical operator '||' OR
try {
	if (std::get<data::BasicType>(left.type()) == data::BasicType::Boolean && std::get<data::BasicType>(right.type()) == data::BasicType::Boolean)
		return data::Value(std::get<bool>(left) || std::get<bool>(right)); // perform logical OR
	else	// unsupported
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid Logical '||' operator between these types.");
} catch (InvalidException e) {
	throw e;
} catch (std::bad_variant_access e) {
		throw InvalidException("InvalidOperation", left.type().toString() + "," + right.type().toString(), "Invalid Logical '||' operator between these types.");
}}

// Logical operators -- end



} // end of namespace pivot::ecs::script::interpreter::builtins
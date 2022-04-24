#include "pivot/script/Builtins.hxx"

#include <iostream>

namespace pivot::ecs::script::interpreter::builtins {

// Builtins assume that the interpreter has called interpreter::validateParams()
// therefore that the parameters are of correct size and types

// Boolean	isPressed(String key)
//		Returns whether or not the keyboard key is pressed
data::Value builtin_isPressed(const std::vector<data::Value> &params) {
	// std::cout << "isPressed(" << std::get<std::string>(params.at(0)) << ")" << std::endl; // debug
	return data::Value(true);
}

// void	print(String str)
//		print the string
data::Value builtin_print(const std::vector<data::Value> &params) {
	// std::cout << "print(" << std::get<std::string>(params.at(0)) << ")" << std::endl; // debug
	std::cout << std::get<std::string>(params.at(0)) << std::endl;
	return data::Value();
}


} // end of namespace pivot::ecs::script::interpreter::builtins
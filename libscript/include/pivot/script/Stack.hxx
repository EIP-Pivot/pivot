#pragma once

#include "pivot/ecs/Core/Data/value.hxx"

#include <unordered_map>
#include <functional>

namespace pivot::ecs::script::interpreter {

struct Variable {
	std::string name;
	bool hasValue = false;
	data::Value value;
	std::unordered_map<std::string, Variable> members;
};

/**
 * \brief	Container for all the variables a system
 * 			holds during the execution of a tick
 * 
 * 	This container allows access, use and modification of the
 * 	variables it stores. This is use by the interpreter to handle
 * 	variables during the execution of a tick.
 */
class Stack {
public:

	Stack();
	~Stack() = default;


	void push(const Variable &var);
	const Variable &find(const std::string &name) const;
	const Variable &find(const std::string &name, const std::unordered_map<std::string, Variable> &where) const;

	inline void clear() noexcept { _stack.clear(); }

private:
	std::unordered_map<std::string, Variable> _stack; /// name to variable
};

} // end of namespace pivot::ecs::script::interpreter
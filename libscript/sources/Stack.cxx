#include "pivot/script/Stack.hxx"
#include "pivot/script/Exceptions.hxx"

namespace pivot::ecs::script::interpreter {

Stack::Stack()
{
}

void Stack::push(const Variable &var) {
	if (_stack.contains(var.name))
		throw InvalidException("Redefinition", var.name, "Redefinition of variable which already exists.");
	_stack.insert_or_assign(var.name, var);
}

const Variable &Stack::find(const std::string &name) const {
	return find(name, _stack);
}

const Variable &Stack::find(const std::string &name, const std::unordered_map<std::string, Variable> &where) const {
	size_t dot = name.find('.');
	if (dot == std::string::npos) {
		if (!where.contains(name))
			throw InvalidException("UnknownVariable", name, "Unknown variable.");
		return where.at(name);
	}
	if (!where.contains(name.substr(0, dot)))
		throw InvalidException("UnknownVariable", name, "Unknown variable.");
	return find(name.substr(dot + 1), where.at(name.substr(0, dot)).members);
}


} // end of namespace pivot::ecs::script::interpreter

#ifndef __SCRIPT__STACK__
#define __SCRIPT__STACK__
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING


#include "pivot/ecs/Core/Scripting/exprtk.hpp"
#include "pivot/ecs/Core/Scripting/Exceptions.hxx"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <any>

namespace pivot::ecs::script {

struct VariableNew {
	data::Type type;
	data::Value value;
	std::string name;
	bool isEntity;
};

class Stack {
public:
	Stack() = default;
	~Stack() = default;

	void pushVar(const std::string &name, const data::Type &type, const data::Value &value, bool isEntity);
	void pushVar(const VariableNew &var);
	void updateVar(const std::string &name, const data::Value &newValue);
	const std::map<std::string,VariableNew> &getVars() const;
	data::Value &getVarValue(const std::string &name);
	data::Record getAsRecord() const;
	void clear();

	bool contains(const std::string &key) const;
	const VariableNew &find(const std::string &key) const;

	void print() const;
	void print(const std::string &name) const;
protected:
	std::map<std::string, VariableNew> _vars; // Name, Value
};

std::vector<std::string> split(const std::string& str, const std::string& delim);

} // end of namespace pivot::ecs::script

#endif
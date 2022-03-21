#include "pivot/ecs/Core/Scripting/Stack.hxx"

using namespace pivot::ecs;
using namespace pivot::ecs::script;

// Stack::Stack() {
// }

// Stack::~Stack() {
// }

void Stack::pushVar(const std::string &name, const data::Type &type, const data::Value &value, bool isEntity) {
	_vars.insert_or_assign(name, VariableNew {.type = type, .value = value, .name = name, .isEntity = isEntity});
}
void Stack::pushVar(const VariableNew &var) {
	_vars.insert_or_assign(var.name, var);	
}

void Stack::updateVar(const std::string &name, const data::Value &value) {
	std::vector<std::string> accessFields = split(name, ".");
	if (accessFields.empty())
		throw UnhandledException("UNKNOWN ERROR in Stack::updateVar(): accesFields is empty");
	size_t accessCursor = 0;
	if (!_vars.contains(accessFields[accessCursor])) {
		std::cerr << "Stack::updateVar(const std::string &name, const data::Value &value):\t unknown name " << name << std::endl;
		return ;
	}
	data::Value *cursor = &(_vars.at(accessFields[accessCursor]).value);
	while (true) {
		try {
			data::Record &r = std::get<data::Record>(*cursor);
			accessCursor++;
			if (!r.contains(accessFields[accessCursor])) {
				std::cerr << "Stack::updateVar(const std::string &name, const data::Value &value):\t unknown field " << accessFields[accessCursor] << " of record " << accessFields[accessCursor - 1] << std::endl;
				return ;
			}
			if (accessCursor == accessFields.size() - 1) {
				if (value.type() != r.at(accessFields[accessCursor]).type())
					throw InvalidTypeException((std::string("Trying to assign ") + value.type().toString() + " to variable with " + r.at(accessFields[accessCursor]).type().toString()).c_str());
				r.at(accessFields[accessCursor]) = value;
				break;
			}
			cursor = &(r.at(accessFields[accessCursor]));
		} catch (std::bad_variant_access e) {
			if (accessCursor == accessFields.size() - 1) {
				if (value.type() != _vars.at(accessFields[accessCursor]).type)
					throw InvalidTypeException((std::string("Trying to assign ") + cursor->type().toString() + " to variable with " + _vars.at(accessFields[accessCursor]).type.toString()).c_str());
				_vars.at(accessFields[accessCursor]).value = value;
				break;
			} else {
				std::cerr << "Stack::updateVar(const std::string &name, const data::Value &value):\t field " << accessFields[accessCursor] << " is not a record." << std::endl;
				break;
			}
		}
	}
}

data::Record Stack::getAsRecord() const {
	data::Record r;
	for (auto &[key, var] : _vars)
		r.insert_or_assign(var.name, var.value);
	return r;
}

const std::map<std::string,VariableNew> &Stack::getVars() const {
	return _vars;
}

data::Value &Stack::getVarValue(const std::string &name) {
	std::vector<std::string> accessFields = split(name, ".");
	if (accessFields.empty())
		throw UnhandledException("UNKNOWN ERROR in Stack::updateVar(): accesFields is empty");
	size_t accessCursor = 0;
	if (!_vars.contains(accessFields[accessCursor]))
		throw UnknownVariableException(("Stack::updateVar(const std::string &name, const data::Value &value):\t unknown name " + name).c_str());
	data::Value *cursor = &(_vars.at(accessFields[accessCursor]).value);
	while (true) {
		try {
			data::Record &r = std::get<data::Record>(*cursor);
			accessCursor++;
			if (!r.contains(accessFields[accessCursor]))
				throw UnknownVariableException(("Stack::updateVar(const std::string &name, const data::Value &value):\t unknown field " + accessFields[accessCursor] + " of record " + accessFields[accessCursor - 1]).c_str());
			if (accessCursor == accessFields.size() - 1)
				return r.at(accessFields[accessCursor]);
			cursor = &(r.at(accessFields[accessCursor]));
		} catch (std::bad_variant_access e) {
			if (accessCursor == accessFields.size() - 1)
				return _vars.at(accessFields[accessCursor]).value;
			else
				throw UnknownVariableException(("Stack::updateVar(const std::string &name, const data::Value &value):\t field " + accessFields[accessCursor] + " is not a record.").c_str());
		}
	}
}

void Stack::clear() {
	_vars.clear();
}

bool Stack::contains(const std::string &key) const {
	return _vars.contains(key);
}

const VariableNew &Stack::find(const std::string &key) const { // assumes contains has been called or throws
	return _vars.at(key);
}


void Stack::print() const {
	for (auto &[key, v] : _vars)
		print(key);
}

void Stack::print(const std::string &name) const {
	if (!_vars.contains(name)) {
		std::cerr << "Stack::print(const std::string &name):\t unknown name " << name << std::endl;
		return ;
	}
	VariableNew v = _vars.at(name);
	std::cout << std::format("{}{} : {}\n", (v.isEntity ? "entity " : "") ,v.name, v.type.toString()) << std::endl;
}

std::vector<std::string> script::split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find_first_of(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + 1;
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

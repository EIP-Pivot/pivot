#include "pivot/script/Stack.hxx"
#include "Logger.hpp"
#include "pivot/script/Exceptions.hxx"

namespace pivot::ecs::script::interpreter
{

Stack::Stack() {}

// Public methods

void Stack::push(const Variable &var)
{
    if (_stack.contains(var.name)) {
        logger.err("ERROR") << " with variable " << var.name;
        throw InvalidException("Stack Redefinition: Redefinition of variable which already exists.");
    }
    _stack.insert_or_assign(var.name, var);
}

const Variable &Stack::find(const std::string &name) const { return find(name, _stack); }

void Stack::setValue(const std::string &name, const data::Value &newVal)
{
    Variable &v = findMut(name, _stack);
    v.hasValue = true;
    v.value = newVal;
}

// Private methods

Variable &Stack::findMut(const std::string &name, std::unordered_map<std::string, Variable> &where)
{
    size_t dot = name.find('.');
    if (dot == std::string::npos) {
        if (!where.contains(name)) {
            logger.err("ERROR") << " with variable " << name;
            throw InvalidException("Stack Find: Unknown Variable.");
        }
        return where.at(name);
    }
    if (!where.contains(name.substr(0, dot))) {
        logger.err("ERROR") << " with variable " << name;
        throw InvalidException("Stack Find: Unknown Variable.");
    }
    return findMut(name.substr(dot + 1), where.at(name.substr(0, dot)).members);
}

const Variable &Stack::find(const std::string &name, const std::unordered_map<std::string, Variable> &where) const
{
    size_t dot = name.find('.');
    if (dot == std::string::npos) {
        if (!where.contains(name)) {
            logger.err("ERROR") << " with variable " << name;
            throw InvalidException("Stack Find: Unknown Variable.");
        }
        return where.at(name);
    }
    if (!where.contains(name.substr(0, dot))) {
        logger.err("ERROR") << " with variable " << name;
        throw InvalidException("Stack Find: Unknown Variable.");
    }
    return find(name.substr(dot + 1), where.at(name.substr(0, dot)).members);
}

}    // end of namespace pivot::ecs::script::interpreter

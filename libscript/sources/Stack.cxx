#include "pivot/script/Stack.hxx"
#include "Logger.hpp"
#include "pivot/script/Exceptions.hxx"

namespace pivot::ecs::script::interpreter
{

Stack::Stack() {}

// Public methods

void Stack::push(const std::string &name, const data::Value &var)
{
    if (_stack.contains(name)) {
        logger.err("ERROR") << " with variable " << name;
        throw InvalidException("Stack Redefinition: Redefinition of variable which already exists.");
    }
    _stack.insert_or_assign(name, var);
}

const data::Value &Stack::find(const std::string &name) const { return find(name, _stack); }

void Stack::setValue(const std::string &name, const data::Value &newVal) { findMut(name, _stack) = newVal; }

// Private methods

data::Value &Stack::findMut(const std::string &name, data::Record &where)
{
    size_t dot = name.find('.');
    if (dot == std::string::npos) {    // base case, final access of variable
        if (!where.contains(name)) {
            logger.err("ERROR") << " with variable " << name;
            throw InvalidException("Stack Find: Unknown Variable.");
        }
        return where.at(name);
    }
    const std::string accessingVar = name.substr(0, dot);
    if (!where.contains(accessingVar)) {    // record does not contain searched variable
        logger.err("ERROR") << " with variable " << name;
        throw InvalidException("Stack Find: Unknown Variable.");
    } else if (!std::holds_alternative<data::Record>(
                   where.at(accessingVar))) {    // record contains variable, but it is not a record
        logger.err("ERROR") << " with variable " << accessingVar << " of type " << where.at(accessingVar).type();
        throw InvalidException("Stack Find: Variable is not a Record");
    }
    return findMut(name.substr(dot + 1),
                   std::get<data::Record>(where.at(accessingVar)));    // recursive call on the rest of the access chain
}

const data::Value &Stack::find(const std::string &name, const data::Record &where) const
{
    size_t dot = name.find('.');
    if (dot == std::string::npos) {    // base case, final access of variable
        if (!where.contains(name)) {
            logger.err("ERROR") << " with variable " << name;
            throw InvalidException("Stack Find: Unknown Variable.");
        }
        return where.at(name);
    }
    const std::string accessingVar = name.substr(0, dot);
    if (!where.contains(accessingVar)) {    // record does not contain searched variable
        logger.err("ERROR") << " with variable " << name;
        throw InvalidException("Stack Find: Unknown Variable.");
    } else if (!std::holds_alternative<data::Record>(
                   where.at(accessingVar))) {    // record contains variable, but it is not a record
        logger.err("ERROR") << " with variable " << accessingVar << " of type " << where.at(accessingVar).type();
        throw InvalidException("Stack Find: Variable is not a Record");
    }
    return find(name.substr(dot + 1),
                std::get<data::Record>(where.at(accessingVar)));    // recursive call on the rest of the access chain
}

}    // end of namespace pivot::ecs::script::interpreter

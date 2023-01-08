#include "pivot/script/Stack.hxx"
#include "pivot/script/Exceptions.hxx"
#include <cpplogger/Logger.hpp>

namespace pivot::ecs::script::interpreter
{

Stack::Stack() {}

// Public methods

void Stack::push(const std::string &name, const data::Value &var)
{
    if (_stack.contains(name)) {
        logger.err() << "Redifinition of variable '" << name << "'";
        throw InvalidException("Stack Redefinition: Redefinition of variable which already exists.");
    }
    _stack.insert_or_assign(name, var);
}

void Stack::pushEntity(const std::string &entityName, Entity entityId,
                       std::span<const component::ComponentRef> components)
{
    if (entityName.empty()) return;

    data::Record entityRecord;

    for (const component::ComponentRef &ref: components) {
        entityRecord.insert_or_assign(ref.description().name, ref.get());
    }
    entityRecord.insert_or_assign("id", data::Value{EntityRef{entityId}});
    this->push(entityName, entityRecord);
}

void Stack::updateEntity(const std::string &entityName, std::span<component::ComponentRef> components)
{
    if (entityName.empty()) return;

    const data::Record newEntityRecord = std::get<data::Record>(this->find(entityName));
    for (component::ComponentRef &ref: components) { ref.set(newEntityRecord.at(ref.description().name)); }
}

const data::Value Stack::find(const std::string &name) const { return find(name, _stack); }

void Stack::setValue(const std::string &name, const data::Value &newVal)
{
    if (!setVectorValue(name, newVal, _stack)) findMut(name, _stack) = newVal;
}

// Private methods

bool Stack::setVectorValue(const std::string &name, const data::Value &newVal, data::Record &where)
{
    size_t dot = name.find('.');
    if (dot == std::string::npos) {    // base case, final access of variable
        return false;
    }
    const std::string accessingVar = name.substr(0, dot);
    if (!where.contains(accessingVar)) {    // record does not contain searched variable
        return false;
    } else if (!std::holds_alternative<data::Record>(
                   where.at(accessingVar))) {    // record contains variable, but it is not a record
        if (std::holds_alternative<glm::vec3>(where.at(accessingVar))) {    // but it might be a vector3
            if (!std::holds_alternative<double>(newVal)) {
                logger.err("ERROR") << " with new variable of type " << newVal.type();
                throw InvalidException("Stack Find: Can only apply Number type to Vector3 field");
            }
            // if it is a vector3, check that the accessed variable is 'x' 'y' or 'z'
            const std::string accessedVar = name.substr(dot + 1);
            if (accessedVar == "x") {
                std::get<glm::vec3>(where.at(accessingVar)).x = std::get<double>(newVal);
                return true;
            } else if (accessedVar == "y") {
                std::get<glm::vec3>(where.at(accessingVar)).y = std::get<double>(newVal);
                return true;
            } else if (accessedVar == "z") {
                std::get<glm::vec3>(where.at(accessingVar)).z = std::get<double>(newVal);
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    return setVectorValue(
        name.substr(dot + 1), newVal,
        std::get<data::Record>(where.at(accessingVar)));    // recursive call on the rest of the access chain
}

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

const data::Value Stack::find(const std::string &name, const data::Record &where) const
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
        if (std::holds_alternative<glm::vec3>(where.at(accessingVar))) {    // but it might be a vector3
            // if it is a vector3, check that the accessed variable is 'x' 'y' or 'z'
            const std::string accessedVar = name.substr(dot + 1);
            if (accessedVar == "x") {
                return (double)std::get<glm::vec3>(where.at(accessingVar)).x;
            } else if (accessedVar == "y") {
                return (double)std::get<glm::vec3>(where.at(accessingVar)).y;
            } else if (accessedVar == "z") {
                return (double)std::get<glm::vec3>(where.at(accessingVar)).z;
            } else {
                logger.err("ERROR") << " with variable " << accessingVar << " of type "
                                    << where.at(accessingVar).type();
                throw InvalidException("Stack Find: Can only access field 'x' 'y' or 'z' of Vector3");
            }
        } else {    // if it is neither a record nor a vector3 throw
            logger.err("ERROR") << " with variable " << accessingVar << " of type " << where.at(accessingVar).type();
            throw InvalidException("Stack Find: Variable is not a Record");
        }
    }
    return find(name.substr(dot + 1),
                std::get<data::Record>(where.at(accessingVar)));    // recursive call on the rest of the access chain
}

}    // end of namespace pivot::ecs::script::interpreter

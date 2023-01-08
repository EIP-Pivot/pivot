#pragma once

#include <functional>
#include <span>
#include <unordered_map>

#include <pivot/ecs/Core/Component/ref.hxx>
#include <pivot/ecs/Core/Data/value.hxx>

namespace pivot::ecs::script::interpreter
{

/*! \brief	Container for all the variables a system holds during the execution of a tick
 *
 * 	This container allows access, use and modification of the
 * 	variables it stores. This is use by the interpreter to handle
 * 	variables during the execution of a tick.
 */
class Stack
{
public:
    Stack();
    ~Stack() = default;

    /// Add a variable to the stack, name is pure (no '.')
    void push(const std::string &name, const data::Value &var);
    /// Add an entity and all its components to the stack
    void pushEntity(const std::string &entityName, Entity entityId, std::span<const component::ComponentRef>);
    /// Update an entity components from its value on the stack
    void updateEntity(const std::string &entityName, std::span<component::ComponentRef>);
    /// Find a variable in the stack (read-only), name can contain access ('.')
    const data::Value find(const std::string &name) const;
    /// Modify the value of a variable in the stack, by name, name can contain access ('.')
    void setValue(const std::string &name, const data::Value &newVal);

    /// Clear the stack
    inline void clear() noexcept { _stack.clear(); }

private:
    data::Record _stack;    /// name to variable

    bool setVectorValue(const std::string &name, const data::Value &newVal, data::Record &where);
    data::Value &findMut(const std::string &name, data::Record &where);
    const data::Value find(const std::string &name, const data::Record &where) const;
};

}    // end of namespace pivot::ecs::script::interpreter

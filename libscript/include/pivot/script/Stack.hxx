#pragma once

#include "pivot/ecs/Core/Data/value.hxx"

#include <functional>
#include <unordered_map>

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
    /// Find a variable in the stack (read-only), name can contain access ('.')
    const data::Value &find(const std::string &name) const;
    /// Modify the value of a variable in the stack, by name, name can contain access ('.')
    void setValue(const std::string &name, const data::Value &newVal);

    /// Clear the stack
    inline void clear() noexcept { _stack.clear(); }

private:
    data::Record _stack;    /// name to variable

    data::Value &findMut(const std::string &name, data::Record &where);
    const data::Value &find(const std::string &name, const data::Record &where) const;
};

}    // end of namespace pivot::ecs::script::interpreter
#pragma once

#include "pivot/ecs/Core/Data/value.hxx"

#include <functional>
#include <unordered_map>

namespace pivot::ecs::script::interpreter
{

struct Variable;

/// Represents a variable in the stack data during a system execution
/// It contains all the names and values representing it
struct Variable {
    /// String representing the name of the variable
    std::string name;
    /// Whether the variable has a value or not
    bool hasValue = false;
    /// The value of the variable
    data::Value value;
    /// A map representing its members/fields (also variables)
    std::unordered_map<std::string, Variable> members;
};

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

    /// Add a variable to the stack
    void push(const Variable &var);
    /// Find a variable in the stack (read-only)
    const Variable &find(const std::string &name) const;
    /// Modify the value of a variable in the stack, by name
    void setValue(const std::string &name, const data::Value &newVal);

    /// Clear the stack
    inline void clear() noexcept { _stack.clear(); }

private:
    std::unordered_map<std::string, Variable> _stack;    /// name to variable

    Variable &findMut(const std::string &name, std::unordered_map<std::string, Variable> &where);
    const Variable &find(const std::string &name, const std::unordered_map<std::string, Variable> &where) const;
};

}    // end of namespace pivot::ecs::script::interpreter
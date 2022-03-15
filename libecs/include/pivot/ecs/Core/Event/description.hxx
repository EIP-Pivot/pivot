#pragma once

#include "pivot/ecs/Core/Component/ref.hxx"
#include "pivot/ecs/Core/Data/type.hxx"
#include "pivot/ecs/Core/Data/value.hxx"
#include <string>
#include <vector>

namespace pivot::ecs::event
{
/** \brief Describes how to use a event
 *
 * Store name, Entities name and a optional payload
 */
struct Description {
    /// Event name
    std::string name;
    /// Entities name
    std::vector<std::string> entities;
    /// Value of the payload
    data::Type payload;

    /// Convenience method used to check if any error exist the component's definition
    void validate() const;

    /// Error returned when the validation of a description fails
    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };
};

using EntityComponents = std::vector<std::vector<component::ComponentRef>>;

/// Carry the event data used in system execution.
struct Event {
    /// Event description
    Description description;
    /// Entities Id
    std::vector<Entity> entities;
    /// Value of payload
    data::Value payload;
};

/// Carry the event data with components built by the system manager.
struct EventWithComponent {
    /// Event Object
    Event event;
    /// Reference of component for all needed entities
    EntityComponents components;
};
}    // namespace pivot::ecs::event
#pragma once

#include "pivot/ecs/Core/Component/ref.hxx"
#include "pivot/ecs/Core/Data/type.hxx"
#include "pivot/ecs/Core/Data/value.hxx"
#include <string>
#include <vector>

namespace pivot::ecs::event
{
struct Description {

    std::string name;

    std::vector<std::string> entities;

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

struct Event {
    Description description;
    std::vector<Entity> entities;
    data::Value payload;
};

struct EventWithComponent {
    Event event;
    EntityComponents components;
};
}    // namespace pivot::ecs::event
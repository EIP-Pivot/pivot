#pragma once

#include "pivot/ecs/Core/Component/array.hxx"
#include "pivot/ecs/Core/Component/combination.hxx"
#include "pivot/ecs/Core/Component/description.hxx"
#include "pivot/ecs/Core/Data/value.hxx"
#include "pivot/ecs/Core/Event/description.hxx"

#include <any>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace pivot::ecs::systems
{

/// Describes how to use a systems
struct Description {

    /// system name
    std::string name;
    /// entity name
    std::string entityName;
    /// list of systeme component
    std::vector<std::string> systemComponents;
    /// When event is emit, the system manager will search all system listening to this event
    event::Description eventListener;
    /// Needed component for event
    std::vector<std::vector<std::string>> eventComponents;
    /// Event provenance
    Provenance provenance;
    /// System function
    std::function<void(const Description &, component::ArrayCombination &, const event::EventWithComponent &)> system;
    /// Check if all needed variable are set
    void validate() const;
    /// Error returned when the validation of a description fails
    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };
};

}    // namespace pivot::ecs::systems

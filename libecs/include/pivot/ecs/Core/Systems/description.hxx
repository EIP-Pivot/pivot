#pragma once

#include "pivot/ecs/Core/Component/description.hxx"
#include "pivot/ecs/Core/Data/value.hxx"
#include "pivot/ecs/Core/Event/description.hxx"
#include "pivot/ecs/Core/Component/array.hxx"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <any>
#include <vector>


namespace pivot::ecs::systems
{

struct Description {

    using systemArgs = std::vector<std::reference_wrapper<component::IComponentArray>>;

    std::string name;

    std::vector<std::string> arguments;

    std::function<void(const Description &, systemArgs &, const event::Event &)> system;

    void validate() const;

    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };
};

}    // namespace pivot::ecs::systems
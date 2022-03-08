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

struct Description {

    using systemArgs = component::ArrayCombination;

    std::string name;

    std::vector<std::string> systemComponents;

    event::Description eventListener;

    std::vector<std::vector<std::string>> eventComponents;

    std::function<void(const Description &, systemArgs &, const event::Event &)> system;

    void validate() const;

    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };
};

}    // namespace pivot::ecs::systems
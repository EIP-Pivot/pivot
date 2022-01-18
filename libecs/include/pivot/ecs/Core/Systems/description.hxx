#pragma once

#include "pivot/ecs/Core/Systems/function_traits.hxx"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <any>
#include <vector>


namespace pivot::ecs::systems
{

struct Description {

    using systemArgs = std::vector<std::vector<std::pair<component::Description, std::any>>>;

    std::string name;

    std::vector<std::string> arguments;

    std::function<void(systemArgs &)> system;

    void validate() const;

    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };
};

}    // namespace pivot::ecs::systems
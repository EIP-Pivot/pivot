#pragma once

#include "pivot/ecs/Core/Component/index.hxx"

#include <functional>
#include <stdexcept>
#include <string>

namespace pivot::ecs::systems
{
struct Description {

    std::string name;

    std::vector<std::string> arguments;

    std::function<void(float, Description)> system;

    void validate() const;

    static Description build_system_description(pivot::ecs::component::GlobalIndex globalIndex, const std::string &name, auto system);

    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };
};
}
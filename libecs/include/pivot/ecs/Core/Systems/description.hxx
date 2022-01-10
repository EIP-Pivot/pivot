#pragma once

#include "pivot/ecs/Core/Systems/function_traits.hxx"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>


namespace pivot::ecs::systems
{

template <typename Fn>
concept SystemFunction = requires
{
    std::is_same_v<typename function_traits<Fn>::result_type, void>;
    function_traits<Fn>::arity > 0;
};

struct Description {

    std::string name;

    std::vector<std::string> arguments;

    void validate() const;

    static Description build_system_description(const std::string &name, SystemFunction auto function)
    {
        Description newSystem {
            .name = name,
        };

        typedef function_traits<decltype(function)> traits;

        for (const auto &arg: traits::getArgsName()) {
            if (!arg.has_value()) throw ValidationError("Component not registered.");
            newSystem.arguments.push_back(arg.value());
        }
        return newSystem;
    }

    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };
};

}    // namespace pivot::ecs::systems
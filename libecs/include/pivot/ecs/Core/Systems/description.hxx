#pragma once

#include "pivot/ecs/Core/Systems/function_traits.hxx"

#include <functional>
#include <stdexcept>
#include <string>
#include <iostream>

namespace pivot::ecs::systems
{

template<typename Fn>
concept SystemFunction = requires 
{
    std::is_same_v<typename function_traits<Fn>::result_type, void>;
    function_traits<Fn>::arity > 0;
};

struct Description {

    std::string name;

    std::vector<std::optional<std::string>> arguments;

    std::function<void(float, Description)> system;

    void validate() const;

    static Description build_system_description(const std::string &name, SystemFunction auto function)
    {
        Description newSystem;
        newSystem.name = name;

        typedef function_traits<decltype(function)> traits;

        const auto &n = traits::getArgsName();
        newSystem.arguments = std::vector(n.begin(), n.end());
        return newSystem;
    }

    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };
};

}
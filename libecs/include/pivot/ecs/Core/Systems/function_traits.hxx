#pragma once

#include <pivot/ecs/Core/Component/index.hxx>

#include <array>
#include <cstddef>
#include <tuple>

template <typename>
struct function_traits;

template <typename T>
struct function_traits : public function_traits<decltype(&T::operator())> {
};

// Specialization for pointers to member function or lambdas
template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const> : function_traits<ReturnType (*)(Args...)> {
};

// Specialization for function pointers
template <typename ReturnType, typename... Args>
struct function_traits<ReturnType (*)(Args...)> {
    static constexpr size_t arity = sizeof...(Args);
    
    using result_type = ReturnType;

    using args_type = std::tuple<Args...>;
    
    template <size_t i>
    struct arg {
        using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
    };

    static std::vector<std::optional<std::string>> getArgsName() {
        std::vector<std::optional<std::string>> args;
        ((args.push_back(pivot::ecs::component::GlobalIndex::getSingleton().getComponentNameByType<Args>())), ...);
        return args;
    }
};
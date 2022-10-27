#pragma once

#include <optional>

#include <cpplogger/Logger.hpp>

#include "pivot/debug.hxx"
#include "pivot/utility/assertion_macros.hxx"
#include "pivot/utility/benchmark.hxx"

#include "pivot/Compiler.hxx"
#include "pivot/Platform.hxx"

namespace pivot
{

template <typename T>
using Ref = std::reference_wrapper<T>;

template <typename T>
using OptionalRef = std::optional<Ref<T>>;

template <class...>
/// Used to error out in constexpr if
constexpr std::false_type always_false{};

template <typename T>
/// Only accept hashable type
concept Hashable = requires(T a)
{
    {
        std::hash<T>{}(a)
        } -> std::convertible_to<std::size_t>;
};

}    // namespace pivot

#define DEBUG_FUNCTION() PROFILE_FUNCTION();

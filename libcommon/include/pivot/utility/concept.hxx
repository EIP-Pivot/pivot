#pragma once

#include <functional>

namespace pivot
{

template <typename T>
/// Only accept hashable type
concept Hashable = requires(T a)
{
    {
        std::hash<T>{}(a)
        } -> std::convertible_to<std::size_t>;
};

}    // namespace pivot

#pragma once

#include <optional>

#include <cpplogger/Logger.hpp>
#include <cpplogger/utils/source_location.hpp>

#ifndef NDEBUG
    #define DEBUG_FUNCTION logger.trace(::function_name()) << "Entered";

    #define pivot_assert(expr, msg) (static_cast<bool>(expr) ? void(0) : __pivot_assert_failed(#expr, msg))

    #define __pivot_assert_failed(expr, msg)   \
        (/* detect catch2 at runtime */ false) \
            ? (void(0))                        \
            : (logger.err(::file_position()) << "Assertion failed: " #expr " :: " << msg, logger.stop(), std::abort())

    #define pivot_check(expr, msg) (static_cast<bool>(expr) ? void(0) : __pivot_check_failed(#expr, msg))

    #define __pivot_check_failed(expr, msg) (logger.warn(::file_position()) << #expr ": " << msg, void(0))

#else
    #define pivot_assert(e) void(0);
    #define pivot_check(e) void(0);
    #define DEBUG_FUNCTION void(0);
#endif

namespace pivot
{

template <typename T>
using Ref = std::reference_wrapper<T>;

template <typename T>
using OptionalRef = std::optional<Ref<T>>;

}    // namespace pivot

#pragma once

#include <optional>

#include <cpplogger/Logger.hpp>
#include <cpplogger/utils/source_location.hpp>

#include "pivot/debug.hxx"

#include "pivot/Compiler.hxx"
#include "pivot/Platform.hxx"

#ifndef NDEBUG
    #define DEBUG_FUNCTION logger.trace(::function_name()) << "Entered";

    #define pivot_assert(expr, msg)                                            \
        {                                                                      \
            using Platform = pivot::Platform;                                  \
            if (UNLIKELY(!(expr))) {                                           \
                __pivot_assert_failed(#expr, msg);                             \
                if (Platform::isDebuggerPresent()) { Platform::breakpoint(); } \
                std::abort();                                                  \
            }                                                                  \
        }

    #define __pivot_assert_failed(expr, msg)                                       \
        logger.err(::file_position()) << "Assertion failed: " #expr " :: " << msg; \
        logger.stop();

    #define pivot_check(expr, msg) \
        ((LIKELY(!!(expr))) || __pivot_check_failed(#expr, msg)) || ([]() { using Platform = pivot::Platform; if (Platform::isDebuggerPresent()) { Platform::breakpoint(); }}(), false)
    #define __pivot_check_failed(expr, msg) (logger.warn(::file_position()) << #expr ": " << msg, false)

#else
    #define pivot_assert(expr, msg) void(0);
    #define pivot_check(expr, msg) (LIKELY(!!(expr)))
    #define DEBUG_FUNCTION void(0);
#endif

namespace pivot
{

template <typename T>
using Ref = std::reference_wrapper<T>;

template <typename T>
using OptionalRef = std::optional<Ref<T>>;

}    // namespace pivot

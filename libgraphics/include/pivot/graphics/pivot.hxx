#pragma once

#include <cstdlib>
#include <filesystem>
#include <source_location>
#include <string>
#include <string_view>

#include <Logger.hpp>

#ifndef PIVOT_SOURCE_DIRECTORY
    #define PIVOT_SOURCE_DIRECTORY "."
#endif

namespace pivot
{

namespace graphics
{

    /// Indicated the number of frame rendered in advance by the engine
    constexpr const auto MaxFrameInFlight = 3;
    static_assert(MaxFrameInFlight >= 1);

}    // namespace graphics

}    // namespace pivot

#ifndef NDEBUG
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
#endif

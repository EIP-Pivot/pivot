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

/// Return the name of the calling function
constexpr std::string_view
function_name(const std::source_location &location = std::source_location::current()) noexcept
{
    return location.function_name();
}

/// @brief Return the position of the calling function
///
/// Format: "filename":"line":"column"
inline std::string file_position(const std::source_location &location = std::source_location::current()) noexcept
{
    return std::string() + std::filesystem::relative(location.file_name(), PIVOT_SOURCE_DIRECTORY).string() + ":" +
           std::to_string(location.line()) + ":" + std::to_string(location.column());
}

namespace graphics
{

    /// Indicated the number of frame rendered in advance by the engine
    constexpr const auto MaxFrameInFlight = 3;
    static_assert(MaxFrameInFlight >= 1);

}    // namespace graphics

}    // namespace pivot

#ifndef NDEBUG
    #define pivot_assert(expr) (static_cast<bool>(expr) ? void(0) : __pivot_assert_failed(#expr))

    #define __pivot_assert_failed(expr)        \
        (/* detect catch2 at runtime */ false) \
            ? (void(0))                        \
            : (logger.err(::pivot::file_position()) << "Assertion failed: " #expr, logger.stop(), std::abort())

#else
    #define pivot_assert(e) void(0);
#endif

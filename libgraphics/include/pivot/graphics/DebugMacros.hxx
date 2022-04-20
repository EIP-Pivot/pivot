#pragma once

#include <Logger.hpp>
#include <source_location>

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
inline std::string_view file_position(const std::source_location &location = std::source_location::current()) noexcept
{
    return std::string() + location.file_name() + ":" + std::to_string(location.line()) + ":" +
           std::to_string(location.column());
}

}    // namespace pivot

#ifdef NDEBUG
    #define DEBUG_FUNCTION
#else
    #define DEBUG_FUNCTION logger LOGGER_ACCESS trace(::pivot::function_name()) << "Entered";
#endif

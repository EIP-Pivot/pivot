#pragma once

#include "pivot/debug.hxx"

#if !defined(PLATFORM_WINDOWS)

static_assert(false, "Windows Platform header included on a non windows platform");

#endif

#include <intrin.h>

namespace pivot
{

///
/// @brief Windows-specific function and commands
///
///
class WindowsPlatform
{
public:
    /// Is a debugger attached to the current process
    static bool isDebuggerPresent();

    /// Break into the debugger
    FORCEINLINE static void breakpoint() { (__nop(), __debugbreak()); }

    ///
    /// @brief Set the Thread Name
    ///
    /// @param thread the handle of the thread to name
    /// @param name the new  name of the thread
    ///
    static void setThreadName(std::jthread &thread, const std::string &name);

    ///
    /// @brief Get the Thread Name
    ///
    /// @param thread the thread handle to get the name of
    /// @return std::string the name of the thread
    ///
    static std::string getThreadName(std::jthread &thread);
};

/// Alias to the current platform
using Platform = WindowsPlatform;

}    // namespace pivot

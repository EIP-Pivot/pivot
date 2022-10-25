#pragma once

#include "pivot/debug.hxx"

#if !defined(PLATFORM_WINDOWS)

static_assert(false, "Windows Platform header included on a non windows platform");

#endif

#include <intrin.h>

namespace pivot
{
class WindowsPlatform
{
public:
    static bool isDebuggerPresent();

    FORCEINLINE static void breakpoint() { (__nop(), __debugbreak()); }

    static void setThreadName(std::jthread &thread, const std::string &name);
    static std::string getThreadName(std::jthread &thread);
};

using Platform = WindowsPlatform;

}    // namespace pivot

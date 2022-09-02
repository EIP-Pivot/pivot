#pragma once

#include "pivot/debug.hxx"

#if !defined(PLATFORM_WINDOWS)

static_assert(false, "Windows Platform header included on a non windows platform");

#endif

namespace pivot
{
class WindowsPlatform
{
public:
    static bool isDebuggerPresent();

    FORCEINLINE static void breakpoint() { (__nop(), __debugbreak()); }
};

using Platform = WindowsPlatform;

}    // namespace pivot

#pragma once

#include "pivot/debug.hxx"

#if !defined(PLATFORM_UNIX)

static_assert(false, "Unix Platform header included on a non Unix platform");

#endif

namespace pivot
{
class UnixPlateform
{
public:
    static bool isDebuggerPresent();

    FORCEINLINE static void breakpoint() { raise(SIGTRAP); }
};

using Platform = UnixPlateform;

}    // namespace pivot

#pragma once

#include <string>
#include <thread>

#include "pivot/debug.hxx"

#if !defined(PLATFORM_UNIX)

static_assert(false, "Unix Platform header included on a non Unix platform");

#endif

namespace pivot
{
class UnixPlateform
{
private:
    constexpr static unsigned UnixThreadNameLimit = 15;

public:
    static bool isDebuggerPresent();

    FORCEINLINE static void breakpoint() { raise(SIGTRAP); }

    static void setThreadName(std::jthread &thread, const std::string &name);
    static std::string getThreadName(std::jthread &thread);
};

using Platform = UnixPlateform;

}    // namespace pivot

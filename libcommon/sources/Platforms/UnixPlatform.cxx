#include "pivot/Platforms/UnixPlatform.hxx"

#if defined(PLATFORM_UNIX)

    #include <fcntl.h>
    #include <iterator>
    #include <string.h>
    #include <sys/utsname.h>
    #include <unistd.h>

namespace pivot
{

bool UnixPlateform::isDebuggerPresent()
{
    // If a process is tracing this one then TracerPid in /proc/self/status will
    // be the id of the tracing process. Use SignalHandler safe functions

    int StatusFile = open("/proc/self/status", O_RDONLY);
    if (StatusFile == -1) {
        // Failed - unknown debugger status.
        return false;
    }

    char Buffer[256];
    ssize_t Length = read(StatusFile, Buffer, sizeof(Buffer));
    close(StatusFile);

    constexpr char TracerString[] = "TracerPid:\t";
    constexpr ssize_t LenTracerString = std::size(TracerString);

    for (int i = 0; (Length - i) > LenTracerString; i++) {
        // TracerPid is found
        if (strncmp(&Buffer[i], TracerString, LenTracerString) == 0) {
            // 0 if no process is tracing.
            return Buffer[i + LenTracerString] != '0';
        }
    }

    return false;
}

}    // namespace pivot

#endif

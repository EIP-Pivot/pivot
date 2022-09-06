#include "pivot/Platforms/UnixPlatform.hxx"

#if defined(PLATFORM_UNIX)

    #include <fcntl.h>
    #include <iterator>
    #include <string.h>
    #include <sys/utsname.h>
    #include <unistd.h>
    #include <cstring>

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
    Buffer[255] = 0;
    ssize_t Length = read(StatusFile, Buffer, sizeof(Buffer) - 1);
    if (Length == -1)  {
        // Failed - unknown debugger status.
        return false;
    }

    close(StatusFile);

    constexpr char TracerString[] = "TracerPid:\t";
    const ssize_t LenTracerString = std::strlen(TracerString);

    const char *foundStr = std::strstr(Buffer, TracerString);

    if (foundStr != nullptr) {
        return foundStr[LenTracerString] != '0';
    } else {
        return false;
    }
}

}    // namespace pivot

#endif

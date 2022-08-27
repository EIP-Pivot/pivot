#include "pivot/Platform.hxx"

#ifdef PLATFORM_UNIX

    #include <fcntl.h>
    #include <string.h>
    #include <sys/utsname.h>
    #include <unistd.h>

    #include <unordered_map>

namespace pivot
{

bool plateform::isDebuggerPresent()
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

    bool bDebugging = false;
    const char *TracerString = "TracerPid:\t";
    const ssize_t LenTracerString = strlen(TracerString);
    int i = 0;

    while ((Length - i) > LenTracerString) {
        // TracerPid is found
        if (strncmp(&Buffer[i], TracerString, LenTracerString) == 0) {
            // 0 if no process is tracing.
            bDebugging = Buffer[i + LenTracerString] != '0';
            break;
        }

        ++i;
    }

    close(StatusFile);
    return bDebugging;
}

}    // namespace pivot

#endif

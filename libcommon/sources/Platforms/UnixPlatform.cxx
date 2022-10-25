#include "pivot/Platforms/UnixPlatform.hxx"

#if defined(PLATFORM_UNIX)

    #include <cstring>
    #include <fcntl.h>
    #include <iterator>
    #include <string.h>
    #include <sys/utsname.h>
    #include <unistd.h>

    #include "pivot/pivot.hxx"

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
    if (Length == -1) {
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

void UnixPlateform::setThreadName(std::jthread &thread, const std::string &name)
{
    std::string sizeLimitedThreadName = name;

    if (sizeLimitedThreadName.size() > UnixThreadNameLimit) {
        constexpr char sDash[] = "-";
        constexpr size_t uDashLen = std::size(sDash);
        constexpr int numToLeave = (UnixThreadNameLimit - uDashLen) / 2;

        sizeLimitedThreadName = name.substr(0, UnixThreadNameLimit - (numToLeave + uDashLen));
        sizeLimitedThreadName += sDash;
        sizeLimitedThreadName += name.substr(numToLeave + UnixThreadNameLimit - (numToLeave + uDashLen), name.size());

        pivotAssert(sizeLimitedThreadName.size() <= UnixThreadNameLimit);
    }

    int errorCode = pthread_setname_np(thread.native_handle(), sizeLimitedThreadName.c_str());
    if (errorCode != 0) {
        logger.err("UnixPlateform::setThreadName") << "pthread_setname_np('" << name << "') failed with error "
                                                   << errorCode << "(" << strerror(errorCode) << ").";
    }
}

std::string UnixPlateform::getThreadName(std::jthread &thread)
{
    char name[UnixThreadNameLimit + 1] = {'\0'};

    int errorCode = pthread_getname_np(thread.native_handle(), name, std::size(name));
    if (errorCode != 0) {
        logger.err("UnixPlateform::getThreadName")
            << "pthread_getname_np() failed with error " << errorCode << "(" << strerror(errorCode) << ").";
    }
    return std::string(name);
}

}    // namespace pivot

#endif

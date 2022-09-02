#pragma once

#include <string>

#include "pivot/debug.hxx"

#if defined(PLATFORM_WINDOWS)
    #include "pivot/Platforms/WindowsPlatform.hxx"
#elif defined(PLATFORM_UNIX)
    #include "pivot/Platforms/UnixPlatform.hxx"
#else
static_assert(false, "Unsuported Platform !")
#endif

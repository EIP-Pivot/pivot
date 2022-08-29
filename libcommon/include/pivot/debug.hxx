#pragma once

#include <signal.h>

/** Branch prediction hints */
#ifndef LIKELY    // Hints compiler that expression is likely to be true
    #if (defined(__clang__) || defined(__GNUC__))
        #define LIKELY(x) __builtin_expect(!!(x), 1)
    #else
        // the additional "!!" is added to silence "warning: equality comparison with exteraneous parenthese" messages
        #define LIKELY(x) (!!(x))
    #endif
#endif

#ifndef UNLIKELY    // Hints compiler that expression is unlikely to be true
    #if (defined(__clang__) || defined(__GNUC__))
        #define UNLIKELY(x) __builtin_expect(!!(x), 0)
    #else
        // the additional "!!" is added to silence "warning: equality comparison with exteraneous parenthese" messages
        #define UNLIKELY(x) (!!(x))
    #endif
#endif

#if (defined(__clang__) || defined(__GNUC__))
    #define FORCEINLINE inline __attribute__((always_inline)) /* Force code to be inline */
    #define FORCENOINLINE __attribute__((noinline))           /* Force code to NOT be inline */
#elif defined(_MSC_VER)
    #define FORCEINLINE __forceinline /* Force code to be inline */
    #define FORCENOINLINE __declspec(noinline)
#else
    #define FORCEINLINE
    #define FORCENOINLINE
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #define PLATFORM_UNIX
#endif

#if defined(PLATFORM_UNIX)
    #define PLATFORM_BREAK() raise(SIGTRAP)
#elif defined(PLATFORM_WINDOWS)
    #define PLATFORM_BREAK() (__nop(), __debugbreak())
#else
    #define PLATFORM_BREAK()
#endif

#pragma once

#include <signal.h>

#if defined(__clang__)
    #define COMPILER_CLANG
#elif defined(__GNUC__)
    #define COMPILER_GNU
#elif defined(_MSC_VER)
    #define COMPILER_MSVC
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #define PLATFORM_UNIX
#endif

/** Branch prediction hints */
#ifndef LIKELY    // Hints compiler that expression is likely to be true
    #if (defined(COMPILER_CLANG) || defined(COMPILER_GNU))
        #define LIKELY(x) __builtin_expect(!!(x), 1)
    #else
        // the additional "!!" is added to silence "warning: equality comparison with exteraneous parenthese" messages
        #define LIKELY(x) (!!(x))
    #endif
#endif

#ifndef UNLIKELY    // Hints compiler that expression is unlikely to be true
    #if (defined(COMPILER_CLANG) || defined(COMPILER_GNU))
        #define UNLIKELY(x) __builtin_expect(!!(x), 0)
    #else
        // the additional "!!" is added to silence "warning: equality comparison with exteraneous parenthese" messages
        #define UNLIKELY(x) (!!(x))
    #endif
#endif

#if (defined(COMPILER_CLANG) || defined(COMPILER_GNU))
    #define FORCEINLINE inline __attribute__((always_inline)) /* Force code to be inline */
    #define FORCENOINLINE __attribute__((noinline))           /* Force code to NOT be inline */
#elif defined(COMPILER_MSVC)
    #define FORCEINLINE __forceinline /* Force code to be inline */
    #define FORCENOINLINE __declspec(noinline)
#else
    #define FORCEINLINE
    #define FORCENOINLINE
#endif

#pragma once

#include "pivot/Compiler.hxx"
#include "pivot/Platform.hxx"

#include <atomic>

#define STR(X) #X

#ifndef NDEBUG
    #define DEBUG_FUNCTION logger.trace(::function_name()) << "Entered";

    #define PIVOT_VERIFY_IMPL(Always, Expression)                                                                    \
        ((LIKELY(!!(Expression))) || ([]() {                                                                         \
                                         static std::atomic_bool bExecuted = false;                                  \
                                         if (!bExecuted && Always) {                                                 \
                                             bExecuted = true;                                                       \
                                             logger.err(::file_position()) << "Assertion failed: " STR(#Expression); \
                                             return pivot::Platform::isDebuggerPresent();                            \
                                         }                                                                           \
                                         return false;                                                               \
                                     }()) &&                                                                         \
                                         ([]() { pivot::Platform::breakpoint(); }(), false))

    #define PIVOT_VERIFY_FORMAT_IMPL(Always, Expression, ...)                                                    \
        ((LIKELY(!!(Expression))) || ([]() {                                                                     \
                                         static std::atomic_bool bExecuted = false;                              \
                                         if (!bExecuted && Always) {                                             \
                                             bExecuted = true;                                                   \
                                             logger.err(::file_position())                                       \
                                                 << "Assertion failed: " STR(#Expression) " :: " << __VA_ARGS__; \
                                             return pivot::Platform::isDebuggerPresent();                        \
                                         }                                                                       \
                                         return false;                                                           \
                                     }()) &&                                                                     \
                                         ([]() { pivot::Platform::breakpoint(); }(), false))

    #define verify(Expression) PIVOT_VERIFY_IMPL(false, Expression)
    #define verifyMsg(Expression, ...) PIVOT_VERIFY_FORMAT_IMPL(false, Expression, __VA_ARGS__)
    #define verifyAlways(Expression) PIVOT_VERIFY_IMPL(true, Expression)
    #define verifyAlwaysMsg(Expression, ...) PIVOT_VERIFY_FORMAT_IMPL(true, Expression, __VA_ARGS__)

    #define PIVOT_ASSERT_IMPL(Always, Expression)                                            \
        {                                                                                    \
            if (UNLIKELY(!(Expression))) {                                                   \
                logger.err(::file_position()) << "Assertion failed: " STR(#Expression);      \
                logger.stop();                                                               \
                if (pivot::Platform::isDebuggerPresent()) { pivot::Platform::breakpoint(); } \
                std::abort();                                                                \
            }                                                                                \
        }

    #define PIVOT_ASSERT_FORMAT_IMPL(Always, Expression, ...)                                                 \
        {                                                                                                     \
            if (UNLIKELY(!(Expression))) {                                                                    \
                logger.err(::file_position()) << "Assertion failed: " STR(#Expression) " :: " << __VA_ARGS__; \
                logger.stop();                                                                                \
                if (pivot::Platform::isDebuggerPresent()) { pivot::Platform::breakpoint(); }                  \
                std::abort();                                                                                 \
            }                                                                                                 \
        }

    #define pivotAssert(Expression) PIVOT_ASSERT_IMPL(false, Expression)
    #define pivotAssertMsg(Expression, ...) PIVOT_ASSERT_FORMAT_IMPL(false, Expression, __VA_ARGS__)
    #define pivotAssertNoEntry()                                             \
        {                                                                    \
            pivotAssertMsg(false, "Enclosing block should never be called"); \
            pivot::Compiler::unreachable();                                  \
        }
    #define pivotAssertNoReentry()                                                            \
        {                                                                                     \
            static std::atomic_bool beenHere##__LINE__ = false;                               \
            pivotAssertMsg(!beenHere##__LINE__, "Enclosing block was called more than once"); \
            beenHere##__LINE__ = true;                                                        \
        }

#else
    #define DEBUG_FUNCTION

    #define verify(Expression) (LIKELY(!!(Expression)))
    #define verifyMsg(Expression, ...) (LIKELY(!!(Expression)))
    #define verifyAlways(Expression) (LIKELY(!!(Expression)))
    #define verifyAlwaysMsg(Expression, ...) (LIKELY(!!(Expression)))

    #define pivotAssert(Expression)
    #define pivotAssertMsg(Expression, ...)
    #define pivotAssertNoEntry()
    #define pivotAssertNoReentry()

#endif

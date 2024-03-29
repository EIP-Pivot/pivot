#pragma once

#include "pivot/Compiler.hxx"
#include "pivot/Platform.hxx"

#include "pivot/utility/define.hxx"

#include <atomic>

#ifndef NDEBUG

    #define PIVOT_VERIFY_IMPL(Capture, Always, Expression, ...)                                                 \
        ((LIKELY(!!(Expression))) || ([Capture]() {                                                             \
                                         static std::atomic_bool bExecuted = false;                             \
                                         if (!bExecuted && Always) {                                            \
                                             bExecuted = true;                                                  \
                                             logger.err(::pivot::utils::function_name())                        \
                                                 << "Assertion failed: " STR(#Expression) __VA_OPT__(" :: " <<) \
                                                        __VA_ARGS__;                                            \
                                             return pivot::Platform::isDebuggerPresent();                       \
                                         }                                                                      \
                                         return false;                                                          \
                                     }()) &&                                                                    \
                                         ([]() { pivot::Platform::breakpoint(); }(), false))

    #define verify(Expression) PIVOT_VERIFY_IMPL(, false, Expression)
    #define verifyMsg(Expression, ...) PIVOT_VERIFY_IMPL(&, false, Expression, ##__VA_ARGS__)
    #define verifyAlways(Expression) PIVOT_VERIFY_IMPL(, true, Expression)
    #define verifyAlwaysMsg(Expression, ...) PIVOT_VERIFY_IMPL(&, true, Expression, ##__VA_ARGS__)

    #define PIVOT_ASSERT_IMPL(Always, Expression, ...)                                          \
        {                                                                                       \
            if (UNLIKELY(!(Expression))) {                                                      \
                logger.err(::pivot::utils::function_name())                                     \
                    << "Assertion failed: " STR(#Expression) __VA_OPT__(" :: " <<) __VA_ARGS__; \
                logger.stop();                                                                  \
                if (pivot::Platform::isDebuggerPresent()) { pivot::Platform::breakpoint(); }    \
                std::abort();                                                                   \
            }                                                                                   \
        }

    #define pivotAssert(Expression) PIVOT_ASSERT_IMPL(false, Expression)
    #define pivotAssertMsg(Expression, ...) PIVOT_ASSERT_IMPL(false, Expression, ##__VA_ARGS__)
    #define pivotAssertNoEntry()                                             \
        {                                                                    \
            pivotAssertMsg(false, "Enclosing block should never be called"); \
            pivot::Compiler::unreachable();                                  \
        }
    #define pivotAssertNoReentry()                                                                                  \
        {                                                                                                           \
            static std::atomic_bool PIVOT_MACRO_EXPENDER(beenHere, __LINE__) = false;                               \
            pivotAssertMsg(!PIVOT_MACRO_EXPENDER(beenHere, __LINE__), "Enclosing block was called more than once"); \
            PIVOT_MACRO_EXPENDER(beenHere, __LINE__) = true;                                                        \
        }

#else

    #define verify(Expression) (LIKELY(!!(Expression)))
    #define verifyMsg(Expression, ...) (LIKELY(!!(Expression)))
    #define verifyAlways(Expression) (LIKELY(!!(Expression)))
    #define verifyAlwaysMsg(Expression, ...) (LIKELY(!!(Expression)))

    #define pivotAssert(Expression)
    #define pivotAssertMsg(Expression, ...)
    #define pivotAssertNoEntry()
    #define pivotAssertNoReentry()

#endif

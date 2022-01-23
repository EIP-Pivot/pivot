#pragma once

#include <Logger.hpp>

#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#ifdef NDEBUG
#define DEBUG_FUNCTION
#else
#define DEBUG_FUNCTION logger LOGGER_ACCESS debug(__PRETTY_FUNCTION__) << "Entered";
#endif

#define PRINT_STRING_ARRAY(BUFFER, MESSAGE, ARRAY) \
    {                                              \
        auto l = BUFFER;                           \
        l << MESSAGE << ": [";                     \
        for (const auto &i: ARRAY) {               \
            l << i;                                \
            if (i != ARRAY.back()) l << ", ";      \
        }                                          \
        l << "]";                                  \
    }

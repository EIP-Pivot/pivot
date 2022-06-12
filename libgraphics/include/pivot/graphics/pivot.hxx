#pragma once

#include <cstdlib>

#include <Logger.hpp>

namespace pivot::graphics
{

/// Indicated the number of frame rendered in advance by the engine
constexpr const auto MaxFrameInFlight = 3;
static_assert(MaxFrameInFlight >= 1);

}    // namespace pivot::graphics

#ifndef NDEBUG
    #define pivot_assert(expr) (static_cast<bool>(expr) ? void(0) : __pivot_assert_failed(#expr, __FILE__, __LINE__))

    #define __pivot_assert_failed(expr, file, line) \
        (/* detect catch2 at runtime */ false)      \
            ? (void(0))                             \
            : (logger.err(#file ":" #line) << "Assertion failed: " expr, std::abort())

#else
    #define pivot_assert(e) void(0);
#endif

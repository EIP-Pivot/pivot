#pragma once

#include <optional>

#include <cpplogger/Logger.hpp>

#include "pivot/debug.hxx"
#include "pivot/utility/assertion_macros.hxx"
#include "pivot/utility/benchmark.hxx"

#include "pivot/Compiler.hxx"
#include "pivot/Platform.hxx"

namespace pivot
{

template <typename T>
using Ref = std::reference_wrapper<T>;

template <typename T>
using OptionalRef = std::optional<Ref<T>>;

}    // namespace pivot

#ifndef NDEBUG

    #define DEBUG_FUNCTION() \
        PROFILE_FUNCTION();  \
        logger.trace(::pivot::utils::function_name()) << "Entered";

#else
    #define DEBUG_FUNCTION PROFILE_FUNCTION();

#endif

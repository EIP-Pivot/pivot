#pragma once

#include <optional>

#include <cpplogger/Logger.hpp>
#include <cpplogger/utils/source_location.hpp>

#include "pivot/debug.hxx"
#include "pivot/utility/assertion_macros.hxx"

#include "pivot/Compiler.hxx"
#include "pivot/Platform.hxx"

namespace pivot
{

template <typename T>
using Ref = std::reference_wrapper<T>;

template <typename T>
using OptionalRef = std::optional<Ref<T>>;

}    // namespace pivot

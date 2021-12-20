#pragma once

#if !defined(CULLING_DEBUG) && !defined(NDEBUG)
#define CULLING_DEBUG
#endif

#include <optional>

template <typename T>
using OptionalRef = std::optional<std::reference_wrapper<T>>;

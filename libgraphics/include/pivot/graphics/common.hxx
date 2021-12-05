#pragma once

#include <optional>

#if !defined(PIVOT_CULLING_DEBUG) && !defined(NDEBUG)
#define PIVOT_CULLING_DEBUG
#endif

#ifndef PIVOT_MAX_OBJECT
#define PIVOT_MAX_OBJECT 5000
#endif

#ifndef PIVOT_MAX_TEXTURES
#define PIVOT_MAX_TEXTURES 1000
#endif

#ifndef PIVOT_MAX_COMMANDS
#define PIVOT_MAX_COMMANDS PIVOT_MAX_OBJECT
#endif

#ifndef PIVOT_MAX_MATERIALS
#define PIVOT_MAX_MATERIALS 100
#endif

#ifndef PIVOT_MAX_FRAME_FRAME_IN_FLIGHT
#define PIVOT_MAX_FRAME_FRAME_IN_FLIGHT 3
#endif

namespace pivot::graphics
{

}    // namespace pivot::graphics

template <typename T>
/// Simple alias for optional reference
using OptionalRef = std::optional<std::reference_wrapper<T>>;
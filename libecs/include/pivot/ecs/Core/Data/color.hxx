#pragma once

#include <array>

namespace pivot::ecs::data
{
/// Value containing a RGBA color
struct Color {
    /// Color data in RGBA format
    std::array<float, 4> rgba;

    /// Compare two color
    auto operator<=>(const Color &rhs) const = default;
};
}    // namespace pivot::ecs::data

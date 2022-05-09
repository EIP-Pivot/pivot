#pragma once

#include <string>

namespace pivot::ecs::data
{
/// Value containing a reference to an asset loaded in the engine
struct Asset {
    /// Name of the asset
    std::string name;

    /// Compare two assets by their name
    auto operator<=>(const Asset &rhs) const = default;
};
}    // namespace pivot::ecs::data

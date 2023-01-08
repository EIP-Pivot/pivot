#pragma once

#include <string>
#include <vector>

namespace pivot::graphics::asset
{

/// @brief A group of model
struct Prefab {
    /// The ids of the composing models
    std::vector<std::string> modelIds;
    /// Equality operator
    bool operator==(const Prefab &) const = default;
};

}    // namespace pivot::graphics::asset

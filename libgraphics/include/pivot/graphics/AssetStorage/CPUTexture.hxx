#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics::asset
{

/// @brief Represent a CPU-side Texture
struct CPUTexture {
    /// The vulkan image containing the texture
    std::vector<std::byte> image;
    /// The size of the texture
    vk::Extent3D size;
    /// Equality operator
    bool operator==(const CPUTexture &) const = default;
};

}    // namespace pivot::graphics::asset

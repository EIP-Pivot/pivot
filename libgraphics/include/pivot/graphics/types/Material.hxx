#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace gpuObject
{

/// @struct Material
///
/// @brief Hold color information for the light calculations
struct Material {
    /// RGBA values for the ambient color
    glm::vec4 ambientColor;
    /// RGBA values for the diffuse color
    glm::vec4 diffuse;
    /// RGBA values for the specular color
    glm::vec4 specular;
};

}    // namespace gpuObject

#pragma once

#include <glm/vec3.hpp>

namespace gpuObject
{

/// @struct Material
///
/// @brief Hold color information for the light calculations
struct Material {
    /// RGB values for the ambient color
    alignas(16) glm::vec3 ambientColor;
    /// RGB values for the diffuse color
    alignas(16) glm::vec3 diffuse;
    /// RGB values for the specular color
    alignas(16) glm::vec3 specular;
};

}    // namespace gpuObject

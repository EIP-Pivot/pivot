#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace gpuObject
{

/// @struct Material
///
/// @brief Hold color information for the light calculations
struct Material {
    /// Shininess of the material
    alignas(4) float shininess;
    /// RGB values for the ambient color
    alignas(16) glm::vec3 ambientColor;
    /// RGB values for the diffuse color
    alignas(16) glm::vec3 diffuse;
    /// RGB values for the specular color
    alignas(16) glm::vec3 specular;
};
static_assert(sizeof(Material) % 4 == 0);
static_assert(sizeof(Material) == (sizeof(float) * 4 * 3) + sizeof(float) * 4);

}    // namespace gpuObject

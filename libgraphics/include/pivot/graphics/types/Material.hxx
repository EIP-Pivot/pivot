#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace pivot::graphics::gpu_object
{

/// @struct Material
/// @brief Represent a GPU side material
struct Material {
    /// Base color of the material, if no baseColorTexture is provided
    alignas(16) glm::vec4 baseColor = glm::vec4(1.0f);
    /// Metalic factor
    alignas(4) float metallic = 1.0f;
    /// roughness factor
    alignas(4) float roughness = 1.0f;
    /// Diffuse texture, use to determine the base color of the fragment
    alignas(4) std::int32_t baseColorTexture = -1;
    /// Metallic/Roughness Texture
    alignas(4) std::int32_t metallicRoughnessTexture = -1;
    /// Normal Texture
    alignas(4) std::int32_t normalTexture = -1;
    /// Occlusion Texture
    alignas(4) std::int32_t occlusionTexture = -1;
    /// Emissive Texture
    alignas(4) std::int32_t emissiveTexture = -1;
};

static_assert(sizeof(Material) % 4 == 0);
static_assert(sizeof(Material) == ((sizeof(float) * 4) + (sizeof(float) * 2) + (sizeof(std::int32_t) * 5) + 4));

}    // namespace pivot::graphics::gpu_object

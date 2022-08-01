#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace pivot::graphics::gpu_object
{

/// @struct Material
/// @brief Represent a GPU side material
struct Material {
    /// Alpha cut off
    alignas(4) float alphaCutOff = 1.0f;
    /// Metalic factor
    alignas(4) float metallicFactor = 1.0f;
    /// roughness factor
    alignas(4) float roughnessFactor = 1.0f;
    /// Base color of the material, if no baseColorTexture is provided
    alignas(16) glm::vec4 baseColor = glm::vec4(1.0f);
    /// Base color Factor
    alignas(16) glm::vec4 baseColorFactor = glm::vec4(1.0f);
    /// Emissive Factor
    alignas(16) glm::vec4 emissiveFactor = glm::vec4(1.0f);
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
    /// Specular Glossiness Texture
    alignas(4) std::int32_t specularGlossinessTexture = -1;
    /// Diffuse Texture
    alignas(4) std::int32_t diffuseTexture = -1;

    /// Equality operator
    bool operator==(const Material &) const = default;
};

static_assert(sizeof(Material) % 4 == 0);
static_assert(sizeof(Material) == ((sizeof(float) * 3) + ((sizeof(float) * 4) * 3) + (sizeof(std::int32_t) * 8) + 4));

}    // namespace pivot::graphics::gpu_object

#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace pivot::graphics::gpu_object
{

/// @struct Material
/// Represent a GPU side material
struct Material {
    /// @cond
    alignas(4) float metallic = 1.0f;
    alignas(4) float roughness = 1.0f;
    alignas(16) glm::vec4 baseColor = glm::vec4(1.0f);
    alignas(4) std::int32_t baseColorTexture = -1;
    alignas(4) std::int32_t metallicRoughnessTexture = -1;
    alignas(4) std::int32_t normalTexture = -1;
    alignas(4) std::int32_t occlusionTexture = -1;
    alignas(4) std::int32_t emissiveTexture = -1;
    /// @endcond
};

static_assert(sizeof(Material) % 4 == 0);
// static_assert(sizeof(Material) == ((sizeof(float) * 2) + (sizeof(float) * 4) + (sizeof(std::int32_t) * 5)));

}    // namespace pivot::graphics::gpu_object

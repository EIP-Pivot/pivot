#pragma once

#include "pivot/graphics/AssetStorage/DefaultRessources.hxx"

#include <glm/vec4.hpp>
#include <string>

namespace pivot::graphics::asset
{

/// @brief Represent a CPU-side material
struct CPUMaterial {
    /// @cond
    float alphaCutOff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    glm::vec4 baseColor = glm::vec4(1.0f);
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    glm::vec4 emissiveFactor = glm::vec4(1.0f);
    std::string baseColorTexture = missing_texture_name;
    std::string metallicRoughnessTexture = "";
    std::string normalTexture = "";
    std::string occlusionTexture = "";
    std::string emissiveTexture = "";
    std::string specularGlossinessTexture = "";
    std::string diffuseTexture = "";
    bool operator==(const CPUMaterial &) const = default;
    ///@endcond
};

}    // namespace pivot::graphics::asset

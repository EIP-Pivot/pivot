#pragma once

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/types/RenderObject.hxx"
#include "pivot/graphics/types/common.hxx"

#include <glm/mat4x4.hpp>
#include <string>

namespace gpuObject
{

/// @struct UniformBufferObject
///
/// @brief Hold all the information of the 3D object
struct UniformBufferObject {
    /// @param info The object information
    /// @param assetStorage The AssetStorage used to get the indexes
    UniformBufferObject(const RenderObject &info, const pivot::graphics::AssetStorage &assetStorage);
    /// The model matrix
    glm::mat4 modelMatrix;
    /// The index of the texture in the buffer
    alignas(16) uint32_t textureIndex = 0;
    /// The index of the material in the buffer
    uint32_t materialIndex = 0;
    /// The index of the bounding box
    uint32_t boundingBoxIndex = 0;
};
}    // namespace gpuObject
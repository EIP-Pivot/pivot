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
    alignas(16) glm::mat4 modelMatrix;
    /// The index of the material in the buffer
    alignas(4) std::uint32_t materialIndex = 0;
    /// The index of the bounding box
    alignas(4) std::uint32_t boundingBoxIndex = 0;
};
}    // namespace gpuObject

static_assert(sizeof(gpuObject::UniformBufferObject) % 4 == 0);
static_assert(sizeof(gpuObject::UniformBufferObject) == sizeof(float) * 4 * 4 + sizeof(std::uint32_t) * 4);

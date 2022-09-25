#pragma once

#include "pivot/graphics/AssetStorage/AssetStorage.hxx"
#include "pivot/graphics/types/RenderObject.hxx"

#include <glm/mat4x4.hpp>
#include <stdint.h>
#include <string>

namespace pivot::graphics::gpu_object
{

/// @struct UniformBufferObject
///
/// @brief Hold all the information of the 3D object
struct UniformBufferObject {
    /// Create a UBO for the provided object
    UniformBufferObject(const Transform &transform, const RenderObject &info, const AssetStorage &assetStorage);
    /// The model matrix
    alignas(16) glm::mat4 modelMatrix;
    /// The index of the material in the buffer
    alignas(4) std::uint32_t materialIndex = 0;
    /// The index of the bounding box
    alignas(4) std::uint32_t boundingBoxIndex = 0;
};

static_assert(sizeof(gpu_object::UniformBufferObject) % 4 == 0);
static_assert(sizeof(gpu_object::UniformBufferObject) == sizeof(float) * 4 * 4 + sizeof(std::uint32_t) * 4);

}    // namespace pivot::graphics::gpu_object

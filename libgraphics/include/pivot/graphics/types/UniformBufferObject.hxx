#pragma once

#include "pivot/graphics/types/Transform.hxx"
#include "pivot/graphics/types/vk_types.hxx"

#include <glm/mat4x4.hpp>
#include <string>
#include <variant>

namespace pivot::graphics
{

/// @class ObjectInformation
///
/// @brief Hold the information of the 3D object, on the CPU-side
struct ObjectInformation {
    /// The object transform
    Transform transform;
    /// The name of the texture to apply on the object
    std::string textureIndex;
    /// The name of the material to use on the object
    std::string materialIndex;
};

namespace gpuObject
{

    /// @struct UniformBufferObject
    ///
    /// @brief Hold all the information of the 3D object
    struct UniformBufferObject {
        /// @param info The object information
        /// @param imageStor The texture storage, used to resolve the name of the texture
        /// @param materialStor The material storage, used to resolve material name
        UniformBufferObject(const ObjectInformation &info, const ImageStorage &imageStor,
                            const MaterialStorage &materialStor);
        /// The model matrix
        glm::mat4 modelMatrix;
        /// The index of the texture in the buffer
        alignas(16) uint32_t textureIndex = 0;
        /// The index of the material in the buffer
        uint32_t materialIndex = 0;
    };
}    // namespace gpuObject

}    // namespace pivot::graphics
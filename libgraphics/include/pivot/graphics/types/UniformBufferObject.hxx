#pragma once

#include "pivot/graphics/types/vk_types.hxx"

#include <glm/mat4x4.hpp>
#include <string>

namespace gpuObject
{
/// @struct Transform
///
/// @brief Hold the model matrices
struct Transform {
    /// The translation matrix
    glm::mat4 translation;
    /// The rotation matrix
    glm::mat4 rotation;
    /// The scale matrix
    glm::mat4 scale;
};
}    // namespace gpuObject

/// @class ObjectInformation
///
/// @brief Hold the information of the 3D object, on the CPU-side
struct ObjectInformation {
    /// The object transform
    gpuObject::Transform transform;
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
    /// The GPU transformation matrices
    Transform transform;
    /// The index of the texture in the buffer
    alignas(16) uint32_t textureIndex = 0;
    /// The index of the material in the buffer
    uint32_t materialIndex = 0;
};
}    // namespace gpuObject
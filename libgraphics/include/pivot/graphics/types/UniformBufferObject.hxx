#pragma once

#include "pivot/graphics/types/vk_types.hxx"

#include <glm/mat4x4.hpp>
#include <string>

/// @class ObjectInformation
///
/// @brief Hold the information of the 3D object, on the CPU-side
struct ObjectInformation {
    /// @brief Contain the position, rotation and scale of the 3D object
    struct Transform {
        /// The object position in 3D space
        glm::vec3 translation;
        /// The object rotation, in radians
        glm::vec3 rotation;
        /// The scale of the object
        glm::vec3 scale;
    };
    /// The object transform
    Transform transform;
    /// The name of the texture to apply on the object
    std::string textureIndex;
    /// The name of the material to use on the object
    std::string materialIndex;
};

namespace gpuObject
{
/// @struct Transform
///
/// @brief Hold the model matrices
struct Transform {
    /// Constructor from the CPU-side transform
    ///
    /// @param cpuTransform The CPU side of the model transformation
    Transform(const ObjectInformation::Transform &cpuTransform);
    /// The translation matrix
    glm::mat4 translation;
    /// The rotation matrix
    glm::mat4 rotation;
    /// The scale matrix
    glm::mat4 scale;
};

/// @struct UniformBufferObject
///
/// @brief Hold all the information of the 3D object
struct UniformBufferObject {
    /// Constructor from the CPU-side object information
    ///
    /// @param info The CPU object information
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
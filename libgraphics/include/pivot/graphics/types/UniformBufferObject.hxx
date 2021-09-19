#pragma once

#include "pivot/graphics/types/vk_types.hxx"

#include <glm/mat4x4.hpp>
#include <string>

/// @class ObjectInformation
struct ObjectInformation {
    struct Transform {
        /// The object position in 3D space
        glm::vec3 translation;
        /// The object rotation, in radians
        glm::vec3 rotation;
        /// The scale of the object
        glm::vec3 scale;
    } transform;
    /// The name of the texture to apply on the object
    std::string textureIndex;
    /// The name of the material to use on the object
    std::string materialIndex;
};

namespace gpuObject
{
struct Transform {
    Transform(const ObjectInformation::Transform &cpuTransform);
    glm::mat4 translation;
    glm::mat4 rotation;
    glm::mat4 scale;
};

struct UniformBufferObject {
    UniformBufferObject(const ObjectInformation &info, const ImageStorage &stor, const MaterialStorage &mat);
    Transform transform;
    alignas(16) uint32_t textureIndex = 0;
    uint32_t materialIndex = 0;
};
}    // namespace gpuObject
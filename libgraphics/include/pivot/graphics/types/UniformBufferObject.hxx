#pragma once

#include "pivot/graphics/types/vk_types.hxx"

#include <glm/mat4x4.hpp>
#include <string>

struct ObjectInformation {
    struct Transform {
        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;
    } transform;
    std::string textureIndex;
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
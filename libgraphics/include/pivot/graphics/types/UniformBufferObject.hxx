#pragma once

#include <glm/mat4x4.hpp>

struct ObjectInformation {
    struct Transform {
        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;
    } transform;
    uint32_t textureIndex = 0;
    uint32_t materialIndex = 0;
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
    UniformBufferObject(const ObjectInformation &info);
    Transform transform;
    alignas(16) uint32_t textureIndex = 0;
    uint32_t materialIndex = 0;
};
}    // namespace gpuObject
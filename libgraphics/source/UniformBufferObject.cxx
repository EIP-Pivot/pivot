#include "pivot/graphics/types/UniformBufferObject.hxx"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iterator>

gpuObject::Transform::Transform(const ObjectInformation::Transform &transform)
    : translation(glm::translate(glm::mat4(1), transform.translation)),
      rotation(glm::toMat4(glm::quat(transform.rotation))),
      scale(glm::scale(glm::mat4(1), transform.scale))
{
}

gpuObject::UniformBufferObject::UniformBufferObject(const ObjectInformation &info, const ImageStorage &stor,
                                                    const MaterialStorage &mat)
    : transform(info.transform),
      textureIndex(std::distance(stor.begin(), stor.find(info.textureIndex))),
      materialIndex(std::distance(mat.begin(), mat.find(info.materialIndex)))
{
}

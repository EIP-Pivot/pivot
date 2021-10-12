#include "pivot/graphics/types/UniformBufferObject.hxx"

#include <glm/gtx/quaternion.hpp>
#include <iterator>

gpuObject::UniformBufferObject::UniformBufferObject(const ObjectInformation &info, const ImageStorage &stor,
                                                    const MaterialStorage &mat)
    : transform(info.transform),
      textureIndex(std::distance(stor.begin(), stor.find(info.textureIndex))),
      materialIndex(std::distance(mat.begin(), mat.find(info.materialIndex)))
{
}

#include "pivot/graphics/types/UniformBufferObject.hxx"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iterator>

gpuObject::UniformBufferObject::UniformBufferObject(const ObjectInformation &info,
                                                    const pivot::graphics::AssetStorage &assetStorage)
    : modelMatrix(info.transform.getModelMatrix()),
      textureIndex(assetStorage.getIndex<pivot::graphics::AssetStorage::Texture>(info.textureIndex)),
      materialIndex(assetStorage.getIndex<Material>(info.materialIndex))
{
}

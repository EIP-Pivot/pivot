#include "pivot/graphics/types/UniformBufferObject.hxx"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iterator>

gpuObject::UniformBufferObject::UniformBufferObject(const RenderObject &obj,
                                                    const pivot::graphics::AssetStorage &assetStorage)
    : modelMatrix(obj.objectInformation.transform.getModelMatrix()),
      textureIndex(assetStorage.getIndex<pivot::graphics::AssetStorage::Texture>(obj.objectInformation.textureIndex)),
      materialIndex(assetStorage.getIndex<Material>(obj.objectInformation.materialIndex)),
      boundingBoxIndex(assetStorage.getIndex<MeshBoundingBox>(obj.meshID))
{
}

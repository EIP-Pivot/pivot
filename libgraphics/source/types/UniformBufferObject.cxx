#include "pivot/graphics/types/UniformBufferObject.hxx"

#define CAT(A, B) A##B
#define CHECK_DEFAULT(ASSET, TYPE)                                                                                   \
    if (obj.objectInformation.CAT(ASSET, Index)) {                                                                   \
        CAT(ASSET, Index) = assetStorage.getIndex<TYPE>(obj.objectInformation.CAT(ASSET, Index).value());            \
    } else if (model.default_##ASSET) {                                                                              \
        CAT(ASSET, Index) = assetStorage.getIndex<TYPE>(model.default_##ASSET.value());                              \
    } else {                                                                                                         \
        throw pivot::graphics::AssetStorage::AssetStorageException("No " #ASSET " defined for " + obj.meshID + "!"); \
    }

gpuObject::UniformBufferObject::UniformBufferObject(const RenderObject &obj,
                                                    const pivot::graphics::AssetStorage &assetStorage)
    : modelMatrix(obj.objectInformation.transform.getModelMatrix())

{
    const auto &model = assetStorage.get<pivot::graphics::AssetStorage::Model>(obj.meshID);

    CHECK_DEFAULT(texture, pivot::graphics::AssetStorage::Texture);
    CHECK_DEFAULT(material, gpuObject::Material);
    boundingBoxIndex = assetStorage.getIndex<MeshBoundingBox>(obj.meshID);
}

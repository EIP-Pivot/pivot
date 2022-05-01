#include "pivot/graphics/types/UniformBufferObject.hxx"
#include "pivot/graphics/types/AABB.hxx"
#include "pivot/graphics/types/Material.hxx"

namespace pivot::graphics::gpu_object
{

UniformBufferObject::UniformBufferObject(const Transform &transform, const RenderObject &obj,
                                         const AssetStorage &assetStorage)
    : modelMatrix(transform.getModelMatrix())

{
    const auto &model = assetStorage.get<AssetStorage::Model>(obj.meshID);

    if (!obj.materialIndex.empty()) {
        materialIndex = assetStorage.getIndex<Material>(obj.materialIndex);
    } else if (model.default_material) {
        materialIndex = assetStorage.getIndex<Material>(model.default_material.value());
    } else {
        materialIndex = assetStorage.getIndex<Material>(AssetStorage::missing_material_name);
    }
    boundingBoxIndex = assetStorage.getIndex<AABB>(obj.meshID);
}

}    // namespace pivot::graphics::gpu_object

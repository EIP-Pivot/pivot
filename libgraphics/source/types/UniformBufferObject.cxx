#include "pivot/graphics/types/UniformBufferObject.hxx"
#include "pivot/graphics/types/Material.hxx"

template <class T>
static inline std::optional<std::uint32_t> getDefault(const pivot::graphics::AssetStorage &stor,
                                                      const std::optional<std::string> &value,
                                                      const std::optional<std::string> &defaultValue)
{
    if (value && !value->empty())
        return stor.getIndex<T>(value.value());
    else if (defaultValue && !defaultValue->empty())
        return stor.getIndex<T>(defaultValue.value());
    else
        return std::nullopt;
}

namespace pivot::graphics::gpu_object
{
UniformBufferObject::UniformBufferObject(const RenderObject &obj, const AssetStorage &assetStorage)
    : modelMatrix(obj.objectInformation.transform.getModelMatrix())

{
    const auto &model = assetStorage.get<AssetStorage::Model>(obj.meshID);

    auto tmpIndex = getDefault<Material>(assetStorage, obj.objectInformation.materialIndex, model.default_material);
    if (!tmpIndex || tmpIndex.value() == std::uint32_t(-1))
        throw AssetStorage::AssetStorageException("Missing material for "
                                                  "obj " +
                                                  obj.meshID);
    materialIndex = tmpIndex.value();
    boundingBoxIndex = assetStorage.getIndex<MeshBoundingBox>(obj.meshID);
}

}    // namespace pivot::graphics::gpu_object

#include "pivot/graphics/types/UniformBufferObject.hxx"

template <class T>
static inline std::optional<std::uint32_t> getDefault(const pivot::graphics::AssetStorage &stor,
                                                      const std::optional<std::string> &value,
                                                      const std::optional<std::string> &defaultValue)
{
    if (value)
        return stor.getIndex<T>(value.value());
    else if (defaultValue)
        return stor.getIndex<T>(defaultValue.value());
    else
        return std::nullopt;
}

gpuObject::UniformBufferObject::UniformBufferObject(const RenderObject &obj,
                                                    const pivot::graphics::AssetStorage &assetStorage)
    : modelMatrix(obj.objectInformation.transform.getModelMatrix())

{
    const auto &model = assetStorage.get<pivot::graphics::AssetStorage::Model>(obj.meshID);

    auto tmpIndex = getDefault<pivot::graphics::AssetStorage::Material>(
        assetStorage, obj.objectInformation.materialIndex, model.default_material);
    if (!tmpIndex || tmpIndex.value() == std::uint32_t(-1))
        throw pivot::graphics::AssetStorage::AssetStorageException("Missing material for "
                                                                   "obj " +
                                                                   obj.meshID);
    materialIndex = tmpIndex.value();
    boundingBoxIndex = assetStorage.getIndex<MeshBoundingBox>(obj.meshID);
}

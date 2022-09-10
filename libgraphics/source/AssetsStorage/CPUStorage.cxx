#include "pivot/graphics/AssetStorage/CPUStorage.hxx"
#include "pivot/graphics/AssetStorage/DefaultRessources.hxx"

#include "pivot/pivot.hxx"

namespace pivot::graphics::asset
{

asset::CPUStorage asset::CPUStorage::default_assets()
{
    CPUStorage storage;
    storage.textureStaging.add(missing_texture_name, default_texture_data);
    storage.materialStaging.add(missing_material_name, {
                                                           .baseColorTexture = missing_material_name,
                                                       });

    storage.materialStaging.add("white", {});

    storage.vertexStagingBuffer.insert(storage.vertexStagingBuffer.end(), quad_mesh_vertices.begin(),
                                       quad_mesh_vertices.end());
    storage.indexStagingBuffer.insert(storage.indexStagingBuffer.end(), quad_mesh_indices.begin(),
                                      quad_mesh_indices.end());

    storage.modelStorage[quad_mesh_id] = {
        .mesh =
            {
                .vertexOffset = 0,
                .vertexSize = static_cast<std::uint32_t>(quad_mesh_vertices.size()),
                .indicesOffset = 0,
                .indicesSize = static_cast<std::uint32_t>(quad_mesh_indices.size()),
            },
        .default_material = std::nullopt,
    };
    return storage;
}

template <typename K, typename V>
std::unordered_map<K, V> merge_map_ignore_dup(const std::unordered_map<K, V> &first,
                                              const std::unordered_map<K, V> &second)
{
    std::unordered_map<K, V> result;
    result.reserve(first.size() + second.size());

    for (const auto &[key, value]: first) { result.emplace(key, value); }
    for (const auto &[key, value]: second) {
        if (result.contains(key)) {
            pivot_assert(first.contains(key), key << " is not the deduplicated buffer.");
            continue;
        }
        result.emplace(key, value);
    }
    pivot_assert(result.size() <= first.size() + second.size(), "The merged CPUStorage is bigger than its sources.");
    return result;
}

void asset::CPUStorage::merge(const asset::CPUStorage &other)
{
#define TEST_CONTAINS(field)           \
    for (const auto &[name, _]: field) \
        if (other.field.contains(name)) throw CPUStorageError("Duplicate key in " #field ": " + name);

    TEST_CONTAINS(modelStorage);
    TEST_CONTAINS(prefabStorage);
    TEST_CONTAINS(textureStaging);
    TEST_CONTAINS(materialStaging);
#undef TEST_CONTAINS

    prefabStorage = merge_map_ignore_dup(prefabStorage, other.prefabStorage);
    modelPaths = merge_map_ignore_dup(modelPaths, other.modelPaths);
    texturePaths = merge_map_ignore_dup(texturePaths, other.texturePaths);

    textureStaging.append(other.textureStaging);
    materialStaging.append(other.materialStaging);
    modelStorage.reserve(other.modelStorage.size());

    auto vertexOffsetStart = vertexStagingBuffer.size();
    auto indexOffsetStart = indexStagingBuffer.size();
    for (const auto &[name, model]: other.modelStorage) {
        auto newMod = model;

        newMod.mesh.vertexOffset += vertexOffsetStart;
        newMod.mesh.indicesOffset += indexOffsetStart;

        modelStorage.emplace(name, newMod);
    }
    vertexStagingBuffer.insert(vertexStagingBuffer.end(), other.vertexStagingBuffer.begin(),
                               other.vertexStagingBuffer.end());
    indexStagingBuffer.insert(indexStagingBuffer.end(), other.indexStagingBuffer.begin(),
                              other.indexStagingBuffer.end());
}

asset::CPUStorage &asset::CPUStorage::operator+=(const asset::CPUStorage &other)
{
    merge(other);
    return *this;
}

}    // namespace pivot::graphics::asset

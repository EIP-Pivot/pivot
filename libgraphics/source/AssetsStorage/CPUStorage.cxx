#include "pivot/graphics/AssetStorage/CPUStorage.hxx"
#include "pivot/graphics/AssetStorage/DefaultRessources.hxx"
#include "pivot/graphics/AssetStorage/Model.hxx"
#include "pivot/pivot.hxx"

namespace pivot::graphics::asset
{

CPUStorage CPUStorage::default_assets()
{
    CPUStorage storage;
    storage.textureStaging.insert(missing_texture_name, default_texture_data);
    storage.materialStaging.insert(missing_material_name, {
                                                              .baseColorTexture = missing_material_name,
                                                          });

    storage.materialStaging.insert("white", {});

    storage.vertexStagingBuffer.insert(storage.vertexStagingBuffer.end(), quad_mesh_vertices.begin(),
                                       quad_mesh_vertices.end());
    storage.indexStagingBuffer.insert(storage.indexStagingBuffer.end(), quad_mesh_indices.begin(),
                                      quad_mesh_indices.end());
    storage.modelStorage[quad_mesh_id] = std::make_shared<ModelNode>(
        quad_mesh_id, Model{
                          .name = quad_mesh_id,
                          .primitives =
                              {
                                  Primitive{
                                      .vertexOffset = 0,
                                      .vertexSize = static_cast<std::uint32_t>(quad_mesh_vertices.size()),
                                      .indicesOffset = 0,
                                      .indicesSize = static_cast<std::uint32_t>(quad_mesh_indices.size()),
                                      .default_material = missing_material_name,
                                      .name = quad_mesh_id,
                                  },
                              },
                          .localMatrix = glm::mat4(1.0f),
                      });
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

void CPUStorage::merge(const CPUStorage &other)
{
#define TEST_CONTAINS(field)           \
    for (const auto &[name, _]: field) \
        if (other.field.contains(name)) throw CPUStorageError("Duplicate key in " #field ": " + name);

    TEST_CONTAINS(modelStorage);
    TEST_CONTAINS(textureStaging);
    TEST_CONTAINS(materialStaging);
#undef TEST_CONTAINS

    modelPaths = merge_map_ignore_dup(modelPaths, other.modelPaths);
    texturePaths = merge_map_ignore_dup(texturePaths, other.texturePaths);

    textureStaging.append(other.textureStaging);
    materialStaging.append(other.materialStaging);
    modelStorage.reserve(other.modelStorage.size());

    auto vertexOffsetStart = vertexStagingBuffer.size();
    auto indexOffsetStart = indexStagingBuffer.size();
    for (const auto &[name, model]: other.modelStorage) {
        for (auto &primitive: model->value.primitives) {
            primitive.vertexOffset += vertexOffsetStart;
            primitive.indicesOffset += indexOffsetStart;
        }
        modelStorage.emplace(name, model);
    }
    vertexStagingBuffer.insert(vertexStagingBuffer.end(), other.vertexStagingBuffer.begin(),
                               other.vertexStagingBuffer.end());
    indexStagingBuffer.insert(indexStagingBuffer.end(), other.indexStagingBuffer.begin(),
                              other.indexStagingBuffer.end());
}

bool CPUStorage::empty() const noexcept
{
    return modelStorage.empty() && vertexStagingBuffer.empty() && indexStagingBuffer.empty() &&
           textureStaging.empty() && materialStaging.empty();
}

CPUStorage &CPUStorage::operator+=(const CPUStorage &other)
{
    merge(other);
    return *this;
}

}    // namespace pivot::graphics::asset

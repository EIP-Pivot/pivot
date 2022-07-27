#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/pivot.hxx"

namespace pivot::graphics
{

const AssetStorage::CPUTexture AssetStorage::default_texture_data{

    .image =
        {
            std::byte(0x00),
            std::byte(0x00),
            std::byte(0x00),
            std::byte(0xff),

            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),

            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),

            std::byte(0x00),
            std::byte(0x00),
            std::byte(0x00),
            std::byte(0xff),
        },
    .size = {2, 2, 1},
};

const std::vector<Vertex> AssetStorage::quad_vertices = {
    {
        .pos = {-0.5f, -0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .texCoord = {-0.5f, -0.5f},
        .color = {1.0f, 0.0f, 0.0f},
        .tangent = {0.0f, 0.0f, 0.0f, 0.0f},
    },
    {
        .pos = {0.5f, -0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .texCoord = {0.5f, -0.5f},
        .color = {0.0f, 1.0f, 0.0f},
        .tangent = {0.0f, 0.0f, 0.0f, 0.0f},
    },
    {
        .pos = {0.5f, 0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .texCoord = {0.5f, 0.5f},
        .color = {0.0f, 0.0f, 1.0f},
        .tangent = {0.0f, 0.0f, 0.0f, 0.0f},
    },
    {
        .pos = {-0.5f, 0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .texCoord = {-0.5f, 0.5f},
        .color = {1.0f, 1.0f, 1.0f},
        .tangent = {0.0f, 0.0f, 0.0f, 0.0f},
    },
};
const std::vector<std::uint32_t> AssetStorage::quad_indices = {0, 1, 2, 2, 3, 0};

AssetStorage::CPUStorage AssetStorage::CPUStorage::default_assets()
{
    auto quad = std::make_shared<ModelNode>(
        quad_mesh, Model{
                       .name = quad_mesh,
                       .primitives =
                           {
                               Primitive{
                                   .vertexOffset = 0,
                                   .vertexSize = static_cast<std::uint32_t>(quad_vertices.size()),
                                   .indicesOffset = 0,
                                   .indicesSize = static_cast<std::uint32_t>(quad_indices.size()),
                                   .default_material = AssetStorage::missing_material_name,
                                   .name = quad_mesh,
                               },
                           },
                       .localMatrix = glm::mat4(1.0f),
                   });
    CPUStorage storage;
    storage.textureStaging.insert(missing_texture_name, default_texture_data);
    storage.materialStaging.insert(missing_material_name, {
                                                              .baseColorTexture = missing_material_name,
                                                          });

    storage.materialStaging.insert("white", {});

    storage.vertexStagingBuffer.insert(storage.vertexStagingBuffer.end(), quad_vertices.begin(), quad_vertices.end());
    storage.indexStagingBuffer.insert(storage.indexStagingBuffer.end(), quad_indices.begin(), quad_indices.end());
    storage.modelStorage[quad_mesh] = quad;
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

void AssetStorage::CPUStorage::merge(const AssetStorage::CPUStorage &other)
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

bool AssetStorage::CPUStorage::empty() const noexcept
{
    return modelStorage.empty() && vertexStagingBuffer.empty() && indexStagingBuffer.empty() &&
           textureStaging.empty() && materialStaging.empty();
}

AssetStorage::CPUStorage &AssetStorage::CPUStorage::operator+=(const AssetStorage::CPUStorage &other)
{
    merge(other);
    return *this;
}

}    // namespace pivot::graphics

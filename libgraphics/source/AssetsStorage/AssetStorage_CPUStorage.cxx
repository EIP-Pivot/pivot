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
    CPUStorage storage;
    storage.textureStaging.add(missing_texture_name, default_texture_data);
    storage.materialStaging.add(missing_material_name, {
                                                           .baseColorTexture = missing_material_name,
                                                       });

    storage.materialStaging.add("white", {});

    storage.vertexStagingBuffer.insert(storage.vertexStagingBuffer.end(), quad_vertices.begin(), quad_vertices.end());
    storage.indexStagingBuffer.insert(storage.indexStagingBuffer.end(), quad_indices.begin(), quad_indices.end());
    storage.modelStorage[quad_mesh] = {
        .mesh =
            {
                .vertexOffset = 0,
                .vertexSize = static_cast<std::uint32_t>(quad_vertices.size()),
                .indicesOffset = 0,
                .indicesSize = static_cast<std::uint32_t>(quad_indices.size()),
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
            pivotAssertMsg(first.contains(key), key << " is not the deduplicated buffer.");
            continue;
        }
        result.emplace(key, value);
    }
    pivotAssertMsg(result.size() <= first.size() + second.size(), "The merged CPUStorage is bigger than its sources.");
    return result;
}

void AssetStorage::CPUStorage::merge(const AssetStorage::CPUStorage &other)
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

AssetStorage::CPUStorage &AssetStorage::CPUStorage::operator+=(const AssetStorage::CPUStorage &other)
{
    merge(other);
    return *this;
}

}    // namespace pivot::graphics

#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <Logger.hpp>

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

void AssetStorage::CPUStorage::merge(const AssetStorage::CPUStorage &other)
{
// Must clone because .merge() require a non const argument
#define MERGE_CONST(field)       \
    auto _##field = other.field; \
    field.merge(_##field);

#define TEST_CONTAINS(field)           \
    for (const auto &[name, _]: field) \
        if (other.field.contains(name)) throw CPUStorageError("Duplicate key in " #field ": " + name);

    TEST_CONTAINS(modelStorage);
    TEST_CONTAINS(prefabStorage);
    TEST_CONTAINS(textureStaging);
    TEST_CONTAINS(materialStaging);
    TEST_CONTAINS(modelPaths);
    TEST_CONTAINS(texturePaths);
#undef TEST_CONTAINS
    MERGE_CONST(prefabStorage);
    MERGE_CONST(modelPaths);
    MERGE_CONST(texturePaths);
#undef MERGE_CONST
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

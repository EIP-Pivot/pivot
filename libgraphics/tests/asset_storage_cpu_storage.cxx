#include "pivot/graphics/AssetStorage.hxx"

#include <catch2/catch_test_macros.hpp>

using namespace pivot::graphics;

TEST_CASE("Panic Texture are loaded", "[assetStorage]")
{
    AssetStorage::CPUStorage storage;
    REQUIRE(storage.texturePaths.empty());

    REQUIRE(storage.textureStaging.size() == 1);
    REQUIRE_NOTHROW(storage.textureStaging.get(AssetStorage::missing_texture_name));
    REQUIRE(storage.textureStaging.get(AssetStorage::missing_texture_name) == AssetStorage::default_texture_data);

    REQUIRE(storage.materialStaging.size() == 2);
    REQUIRE_NOTHROW(storage.materialStaging.get("white"));
    REQUIRE(storage.materialStaging.get("white") == AssetStorage::CPUMaterial{});

    REQUIRE_NOTHROW(storage.materialStaging.get(AssetStorage::missing_material_name));
    REQUIRE(storage.materialStaging.get(AssetStorage::missing_material_name) ==
            AssetStorage::CPUMaterial{
                .baseColorTexture = AssetStorage::missing_material_name,
            });

    REQUIRE(storage.modelStorage.size() == 1);
    REQUIRE_NOTHROW(storage.modelStorage.at(AssetStorage::quad_mesh));
    REQUIRE(storage.modelStorage.at(AssetStorage::quad_mesh) ==
            AssetStorage::Model{
                .mesh =
                    {
                        .vertexOffset = 0,
                        .vertexSize = static_cast<uint32_t>(AssetStorage::quad_vertices.size()),
                        .indicesOffset = 0,
                        .indicesSize = static_cast<uint32_t>(AssetStorage::quad_indices.size()),
                    },
                .default_material = std::nullopt,
            });

    REQUIRE(storage.vertexStagingBuffer == AssetStorage::quad_vertices);
    REQUIRE(storage.indexStagingBuffer == AssetStorage::quad_indices);
}

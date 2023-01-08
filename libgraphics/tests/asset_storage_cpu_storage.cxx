#include "pivot/graphics/AssetStorage/CPUStorage.hxx"
#include "pivot/graphics/AssetStorage/DefaultRessources.hxx"

#include <catch2/catch_test_macros.hpp>

using namespace pivot::graphics;

TEST_CASE("Panic Texture are loaded", "[assetStorage]")
{
    auto storage = asset::CPUStorage::default_assets();
    REQUIRE(storage.texturePaths.empty());

    REQUIRE(storage.textureStaging.size() == 1);
    REQUIRE_NOTHROW(storage.textureStaging.get(asset::missing_texture_name));
    REQUIRE(storage.textureStaging.get(asset::missing_texture_name) == asset::default_texture_data);

    REQUIRE(storage.materialStaging.size() == 2);
    REQUIRE_NOTHROW(storage.materialStaging.get("white"));
    REQUIRE(storage.materialStaging.get("white") == asset::CPUMaterial{});

    REQUIRE_NOTHROW(storage.materialStaging.get(asset::missing_material_name));
    REQUIRE(storage.materialStaging.get(asset::missing_material_name) ==
            asset::CPUMaterial{
                .baseColorTexture = asset::missing_material_name,
            });

    REQUIRE(storage.modelStorage.size() == 1);
    REQUIRE_NOTHROW(storage.modelStorage.at(asset::quad_mesh_id));
    REQUIRE(storage.modelStorage.at(asset::quad_mesh_id) ==
            asset::Model{
                .mesh =
                    {
                        .vertexOffset = 0,
                        .vertexSize = static_cast<uint32_t>(asset::quad_mesh_vertices.size()),
                        .indicesOffset = 0,
                        .indicesSize = static_cast<uint32_t>(asset::quad_mesh_indices.size()),
                    },
                .default_material = std::nullopt,
            });

    REQUIRE(storage.vertexStagingBuffer == asset::quad_mesh_vertices);
    REQUIRE(storage.indexStagingBuffer == asset::quad_mesh_indices);
}

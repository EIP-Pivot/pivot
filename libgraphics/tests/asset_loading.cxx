#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/VulkanApplication.hxx"

#ifndef BASE_PATH
    #define BASE_PATH "./"
#endif

const std::string pathToGltf = BASE_PATH "test_models/basic_triangle.gltf";
const std::string pathToObj = BASE_PATH "test_models/basic_cube.obj";

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

TEST_CASE("loadGltfFile", "[assetStorage]")
{
    AssetStorage::CPUStorage storage;

    SECTION("Return false when incorrect path") REQUIRE_FALSE(loaders::loadGltfModel(pathToObj, storage));

    REQUIRE(loaders::loadGltfModel(pathToGltf, storage));

    REQUIRE_NOTHROW(storage.prefabStorage.at("basic_triangle"));
    const auto &prefab = storage.prefabStorage.at("basic_triangle");
    REQUIRE(prefab.modelIds.size() == 1);

    const auto offset = AssetStorage::quad_vertices.size();

    // Models are stored with "name + vertex offset". Because this model does not have named node, it is only "vertex
    // offset".
    REQUIRE_NOTHROW(storage.modelStorage.at(std::to_string(offset)));
    const auto &model = storage.modelStorage.at(std::to_string(offset));
    REQUIRE(model.default_material.has_value());
    REQUIRE(model.default_material == "white");
    REQUIRE(model.mesh.vertexOffset == AssetStorage::quad_vertices.size());
    REQUIRE(model.mesh.vertexSize == 3);
    REQUIRE(model.mesh.indicesOffset == AssetStorage::quad_indices.size());
    REQUIRE(model.mesh.indicesSize == 3);

    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).pos.x == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).pos.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).pos.z == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).normal == glm::vec3(0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).color == glm::vec4(1.0f));
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).texCoord == glm::vec2(0.0f));

    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).pos.x == 1.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).pos.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).pos.z == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).normal == glm::vec3(0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).color == glm::vec4(1.0f));
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).texCoord == glm::vec2(0.0f));

    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).pos.x == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).pos.y == 1.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).pos.z == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).normal == glm::vec3(0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).color == glm::vec4(1.0f));
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).texCoord == glm::vec2(0.0f));
}

TEST_CASE("loadObjFile", "[assetStorage]")
{

    AssetStorage::CPUStorage storage;

    SECTION("Return false when incorrect path") { REQUIRE_FALSE(loaders::loadObjModel(pathToGltf, storage)); }

    REQUIRE(loaders::loadObjModel(pathToObj, storage));

    const auto &prefab = storage.prefabStorage.at("basic_cube");
    REQUIRE(prefab.modelIds.size() == 1);

    const auto &model = storage.modelStorage.at("square");
    REQUIRE(model.default_material.has_value());
    REQUIRE(model.default_material == "white");
    REQUIRE(model.mesh.vertexOffset == AssetStorage::quad_vertices.size());
    REQUIRE(model.mesh.vertexSize == 4);
    REQUIRE(model.mesh.indicesOffset == AssetStorage::quad_indices.size());
    REQUIRE(model.mesh.indicesSize == 4);

    const auto offset = AssetStorage::quad_vertices.size();

    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).pos.x == -50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).pos.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).pos.z == 50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).color.x == 0.1f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).color.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).color.z == 0.2f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).texCoord.x == -10.f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 0).texCoord.y == 10.f);

    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).pos.x == 50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).pos.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).pos.z == 50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).color.x == 1.f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).color.y == 0.2f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).color.z == 0.3f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).texCoord.x == 10.f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 1).texCoord.y == 10.f);

    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).pos.x == 50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).pos.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).pos.z == -50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).color.x == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).color.y == 0.5f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).color.z == 1.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).texCoord.x == 10.f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 2).texCoord.y == -10.f);

    REQUIRE(storage.vertexStagingBuffer.at(offset + 3).pos.x == -50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 3).pos.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 3).pos.z == -50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 3).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(offset + 3).color.x == 0.1f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 3).color.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 3).color.z == 1.0f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 3).texCoord.x == -10.f);
    REQUIRE(storage.vertexStagingBuffer.at(offset + 3).texCoord.y == -10.f);
}

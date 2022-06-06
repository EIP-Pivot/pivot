#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>

#include "pivot/graphics/AssetStorage.hxx"

#ifndef BASE_PATH
    #define BASE_PATH "./"
#endif

const std::string pathToGltf = BASE_PATH "test_models/boom_box/BoomBox.gltf";
const std::string pathToObj = BASE_PATH "test_models/basic_cube.obj";

using namespace pivot::graphics;

TEST_CASE("loadObjFile", "[assetStorage]")
{

    AssetStorage::CPUStorage storage;

    SECTION("Return false when incorrect path") { REQUIRE_FALSE(loaders::loadObjModel(pathToGltf, storage)); }

    REQUIRE(loaders::loadObjModel(pathToObj, storage));

    const auto &prefab = storage.prefabStorage.at("basic_cube");
    REQUIRE(prefab.modelIds.size() == 1);

    const auto &model = storage.modelStorage.at("square" + std::to_string(AssetStorage::quad_vertices.size()));
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

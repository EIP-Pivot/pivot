#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>

#include "pivot/graphics/AssetStorage/AssetStorage.hxx"

#ifndef BASE_PATH
    #define BASE_PATH "./"
#endif

const std::string pathToGltf = BASE_PATH "test_models/boom_box/BoomBox.gltf";
const std::string pathToObj = BASE_PATH "test_models/basic_cube.obj";

using namespace pivot::graphics;

TEST_CASE("loadObjFile", "[assetStorage]")
{

    asset::CPUStorage storage;

    SECTION("Return false when incorrect path") { REQUIRE_FALSE(loaders::loadObjModel(pathToGltf).has_value()); }

    REQUIRE_NOTHROW(storage = loaders::loadObjModel(pathToObj).value());

    REQUIRE_NOTHROW(storage.modelStorage.at("basic_cube"));
    const auto &prefab = storage.modelStorage.at("basic_cube");
    REQUIRE(prefab->size() == 1);

    REQUIRE_NOTHROW(storage.modelStorage.at("square0"));
    const auto &model = storage.modelStorage.at("square0");
    // REQUIRE(model.default_material.has_value());
    // REQUIRE(model.default_material == "white");
    // REQUIRE(model.mesh.vertexOffset == 0);
    // REQUIRE(model.mesh.vertexSize == 4);
    // REQUIRE(model.mesh.indicesOffset == 0);
    // REQUIRE(model.mesh.indicesSize == 4);

    REQUIRE(storage.vertexStagingBuffer.at(0).pos.x == -50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(0).pos.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(0).pos.z == 50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(0).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(0).color.x == 0.1f);
    REQUIRE(storage.vertexStagingBuffer.at(0).color.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(0).color.z == 0.2f);
    REQUIRE(storage.vertexStagingBuffer.at(0).texCoord.x == -10.f);
    REQUIRE(storage.vertexStagingBuffer.at(0).texCoord.y == 10.f);

    REQUIRE(storage.vertexStagingBuffer.at(1).pos.x == 50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(1).pos.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(1).pos.z == 50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(1).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(1).color.x == 1.f);
    REQUIRE(storage.vertexStagingBuffer.at(1).color.y == 0.2f);
    REQUIRE(storage.vertexStagingBuffer.at(1).color.z == 0.3f);
    REQUIRE(storage.vertexStagingBuffer.at(1).texCoord.x == 10.f);
    REQUIRE(storage.vertexStagingBuffer.at(1).texCoord.y == 10.f);

    REQUIRE(storage.vertexStagingBuffer.at(2).pos.x == 50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(2).pos.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(2).pos.z == -50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(2).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(2).color.x == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(2).color.y == 0.5f);
    REQUIRE(storage.vertexStagingBuffer.at(2).color.z == 1.0f);
    REQUIRE(storage.vertexStagingBuffer.at(2).texCoord.x == 10.f);
    REQUIRE(storage.vertexStagingBuffer.at(2).texCoord.y == -10.f);

    REQUIRE(storage.vertexStagingBuffer.at(3).pos.x == -50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(3).pos.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(3).pos.z == -50.0f);
    REQUIRE(storage.vertexStagingBuffer.at(3).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(storage.vertexStagingBuffer.at(3).color.x == 0.1f);
    REQUIRE(storage.vertexStagingBuffer.at(3).color.y == 0.0f);
    REQUIRE(storage.vertexStagingBuffer.at(3).color.z == 1.0f);
    REQUIRE(storage.vertexStagingBuffer.at(3).texCoord.x == -10.f);
    REQUIRE(storage.vertexStagingBuffer.at(3).texCoord.y == -10.f);
}

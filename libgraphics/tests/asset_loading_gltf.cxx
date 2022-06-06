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

#define VERTEX_CHECK_VECTOR_(index, champ, field, value) \
    REQUIRE(storage.vertexStagingBuffer.at(index).champ.field == value);

#define VERTEX_CHECK_VEC2(index, champ, x_, y_) \
    VERTEX_CHECK_VECTOR_(index, champ, x, x_)   \
    VERTEX_CHECK_VECTOR_(index, champ, y, y_)

#define VERTEX_CHECK_VEC3(index, champ, x_, y_, z_) \
    VERTEX_CHECK_VEC2(index, champ, x_, y_)         \
    VERTEX_CHECK_VECTOR_(index, champ, z, z_)

#define VERTEX_CHECK_VEC4(index, champ, x_, y_, z_, w_) \
    VERTEX_CHECK_VEC3(index, champ, x_, y_, z_)         \
    VERTEX_CHECK_VECTOR_(index, champ, w, w_)

TEST_CASE("loadGltfFile", "[assetStorage]")
{
    static_assert(std::numeric_limits<float>::epsilon() < 1.f);

    const auto offset = AssetStorage::quad_vertices.size();
    AssetStorage::CPUStorage storage;

    SECTION("Return false when incorrect path") REQUIRE_FALSE(loaders::loadGltfModel(pathToObj, storage));

    REQUIRE(loaders::loadGltfModel(pathToGltf, storage));

    std::string testIds{"BoomBox" + std::to_string(offset)};
    REQUIRE_NOTHROW(storage.prefabStorage.at("BoomBox"));
    const auto &prefab = storage.prefabStorage.at("BoomBox");
    REQUIRE(prefab.modelIds.size() == 1);
    REQUIRE(prefab.modelIds.at(0) == testIds);

    REQUIRE(storage.materialStaging.size() == 3);
    REQUIRE_NOTHROW(storage.materialStaging.get("BoomBox_Mat"));
    const auto &mat = storage.materialStaging.get("BoomBox_Mat");
    REQUIRE(mat.baseColor == glm::vec4(1.0f));
    REQUIRE(mat.baseColorFactor == glm::vec4(1.0f));
    REQUIRE(mat.metallicFactor == 1.0f);
    REQUIRE(mat.roughnessFactor == 1.0f);
    REQUIRE(mat.emissiveFactor == glm::vec4(1.0f));
    REQUIRE(mat.baseColorTexture == "BoomBox_baseColor");
    REQUIRE(mat.metallicRoughnessTexture == "BoomBox_occlusionRoughnessMetallic");
    REQUIRE(mat.normalTexture == "BoomBox_normal");
    REQUIRE(mat.occlusionTexture == "BoomBox_occlusionRoughnessMetallic");
    REQUIRE(mat.emissiveTexture == "BoomBox_emissive");

    REQUIRE_NOTHROW(storage.modelStorage.at(testIds));
    const auto &model = storage.modelStorage.at(testIds);
    REQUIRE(model.default_material.has_value());
    REQUIRE(model.default_material == "BoomBox_Mat");
    REQUIRE(model.mesh.vertexOffset == AssetStorage::quad_vertices.size());
    REQUIRE(model.mesh.vertexSize == 3575);
    REQUIRE(model.mesh.indicesOffset == AssetStorage::quad_indices.size());
    REQUIRE(model.mesh.indicesSize == 18108);

    REQUIRE(storage.vertexStagingBuffer.size() == 3575 + AssetStorage::quad_vertices.size());
    REQUIRE(storage.indexStagingBuffer.size() == 18108 + AssetStorage::quad_indices.size());

    using limit = std::numeric_limits<float>;
    logger.debug("position") << std::setprecision(limit::max_digits10)
                             << storage.vertexStagingBuffer.at(offset + 1000).pos.x;
    logger.debug("position") << std::setprecision(limit::max_digits10)
                             << storage.vertexStagingBuffer.at(offset + 1000).pos.y;
    logger.debug("position") << std::setprecision(limit::max_digits10)
                             << storage.vertexStagingBuffer.at(offset + 1000).pos.z;

    logger.debug("normal") << std::setprecision(limit::max_digits10)
                           << storage.vertexStagingBuffer.at(offset + 1000).normal.x;
    logger.debug("normal") << std::setprecision(limit::max_digits10)
                           << storage.vertexStagingBuffer.at(offset + 1000).normal.y;
    logger.debug("normal") << std::setprecision(limit::max_digits10)
                           << storage.vertexStagingBuffer.at(offset + 1000).normal.z;

    logger.debug("color") << std::setprecision(limit::max_digits10)
                          << storage.vertexStagingBuffer.at(offset + 1000).color.x;
    logger.debug("color") << std::setprecision(limit::max_digits10)
                          << storage.vertexStagingBuffer.at(offset + 1000).color.y;
    logger.debug("color") << std::setprecision(limit::max_digits10)
                          << storage.vertexStagingBuffer.at(offset + 1000).color.z;
    logger.debug("color") << std::setprecision(limit::max_digits10)
                          << storage.vertexStagingBuffer.at(offset + 1000).color.w;

    logger.debug("texCoord") << std::setprecision(limit::max_digits10)
                             << storage.vertexStagingBuffer.at(offset + 1000).texCoord.x;
    logger.debug("texCoord") << std::setprecision(limit::max_digits10)
                             << storage.vertexStagingBuffer.at(offset + 1000).texCoord.y;

    logger.debug("tangent") << std::setprecision(limit::max_digits10)
                            << storage.vertexStagingBuffer.at(offset + 1000).tangent.x;
    logger.debug("tangent") << std::setprecision(limit::max_digits10)
                            << storage.vertexStagingBuffer.at(offset + 1000).tangent.y;
    logger.debug("tangent") << std::setprecision(limit::max_digits10)
                            << storage.vertexStagingBuffer.at(offset + 1000).tangent.z;
    logger.debug("tangent") << std::setprecision(limit::max_digits10)
                            << storage.vertexStagingBuffer.at(offset + 1000).tangent.w;

    VERTEX_CHECK_VEC3(offset + 0, pos, 0.00247455505f, -0.00207684329f, 0.00687769148f);
    VERTEX_CHECK_VEC3(offset + 0, normal, -0.662998438f, -0.244545713f, 0.0187361445f);
    VERTEX_CHECK_VEC4(offset + 0, color, 1.0f, 1.0f, 1.0f, 1.0f);
    VERTEX_CHECK_VEC2(offset + 0, texCoord, 0.0681650937f, 0.192196429f);
    VERTEX_CHECK_VEC4(offset + 0, tangent, -0.295275331f, -0.835973203f, -0.462559491f, 1.00000f);

    VERTEX_CHECK_VEC3(offset + 1000, pos, 0.00661812071f, -0.00235924753f, 0.00288167689f);
    VERTEX_CHECK_VEC3(offset + 1000, normal, 0.589514494f, 0.0575772263f, -0.38584733f);
    VERTEX_CHECK_VEC4(offset + 1000, color, 1.0f, 1.0f, 1.0f, 1.0f);
    VERTEX_CHECK_VEC2(offset + 1000, texCoord, 0.90863955f, 0.12404108f);
    VERTEX_CHECK_VEC4(offset + 1000, tangent, -0.114067227f, -0.993130505f, 0.0260791834f, 1.00000f);
}

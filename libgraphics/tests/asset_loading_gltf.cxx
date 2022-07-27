#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <fstream>
#include <iostream>

#include "pivot/graphics/AssetStorage.hxx"

#ifndef BASE_PATH
    #define BASE_PATH "./"
#endif

const std::string pathToGltf = BASE_PATH "test_models/boom_box/BoomBox.gltf";
const std::string pathToOtherGltf = BASE_PATH "test_models/basic_triangle.gltf";

const std::string pathToObj = BASE_PATH "test_models/basic_cube.obj";

using namespace pivot::graphics;

#define VERTEX_CHECK_VECTOR_(stor, index, champ, field, value) \
    REQUIRE(stor.vertexStagingBuffer.at(index).champ.field ==  \
            Catch::Approx(value).epsilon(std::numeric_limits<float>::epsilon()));

#define VERTEX_CHECK_VEC2(stor, index, champ, x_, y_) \
    VERTEX_CHECK_VECTOR_(stor, index, champ, x, x_)   \
    VERTEX_CHECK_VECTOR_(stor, index, champ, y, y_)

#define VERTEX_CHECK_VEC3(stor, index, champ, x_, y_, z_) \
    VERTEX_CHECK_VEC2(stor, index, champ, x_, y_)         \
    VERTEX_CHECK_VECTOR_(stor, index, champ, z, z_)

#define VERTEX_CHECK_VEC4(stor, index, champ, x_, y_, z_, w_) \
    VERTEX_CHECK_VEC3(stor, index, champ, x_, y_, z_)         \
    VERTEX_CHECK_VECTOR_(stor, index, champ, w, w_)

using limit = std::numeric_limits<float>;

#define VERTEX_PRINT_VECTOR_(stor, index, champ, field) \
    logger.debug(#index ":" #champ "." #field)          \
        << std::setprecision(limit::max_digits10) << stor.vertexStagingBuffer.at(index).champ.field;

#define VERTEX_PRINT_VEC2(stor, index, champ)    \
    VERTEX_PRINT_VECTOR_(stor, index, champ, x); \
    VERTEX_PRINT_VECTOR_(stor, index, champ, y)

#define VERTEX_PRINT_VEC3(stor, index, champ) \
    VERTEX_PRINT_VEC2(stor, index, champ);    \
    VERTEX_PRINT_VECTOR_(stor, index, champ, z)

#define VERTEX_PRINT_VEC4(stor, index, champ) \
    VERTEX_PRINT_VEC3(stor, index, champ);    \
    VERTEX_PRINT_VECTOR_(stor, index, champ, w)

#define PRINT_VERTEX(stor, index)                \
    VERTEX_PRINT_VEC3(storage, index, pos);      \
    VERTEX_PRINT_VEC3(storage, index, normal);   \
    VERTEX_PRINT_VEC2(storage, index, texCoord); \
    VERTEX_PRINT_VEC3(storage, index, color);    \
    VERTEX_PRINT_VEC4(storage, index, tangent);

TEST_CASE("loadGltfFile", "[assetStorage]")
{
    static_assert(std::numeric_limits<float>::epsilon() < 1.f);

    AssetStorage::CPUStorage storage;

    SECTION("Return false when incorrect path") REQUIRE_FALSE(loaders::loadGltfModel(pathToObj).has_value());

    REQUIRE_NOTHROW(storage = loaders::loadGltfModel(pathToGltf).value());

    std::string testIds = "BoomBox0";
    REQUIRE_NOTHROW(storage.modelStorage.at("BoomBox"));
    const auto &prefab = storage.modelStorage.at("BoomBox");
    REQUIRE(prefab->size() == 1);

    REQUIRE(storage.materialStaging.size() == 1);
    REQUIRE_NOTHROW(storage.materialStaging.at("BoomBox_Mat"));
    const auto &mat = storage.materialStaging.at("BoomBox_Mat");
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
    for (const auto &primitive: model->value.primitives) {
        REQUIRE(primitive.default_material.has_value());
        REQUIRE(primitive.default_material == "BoomBox_Mat");
    }
    REQUIRE(model->value.primitives.at(0).vertexOffset == 0);
    REQUIRE(model->value.primitives.at(0).vertexSize == 3575);
    REQUIRE(model->value.primitives.at(0).indicesOffset == 0);
    REQUIRE(model->value.primitives.at(0).indicesSize == 18108);

    REQUIRE(storage.vertexStagingBuffer.size() == 3575);
    REQUIRE(storage.indexStagingBuffer.size() == 18108);

    PRINT_VERTEX(storage, 0);
    VERTEX_CHECK_VEC3(storage, 0, pos, 0.00247455505f, -0.00207684329f, 0.00687769148f);
    VERTEX_CHECK_VEC3(storage, 0, normal, -0.937883496f, -0.345936537f, 0.0265043154f);
    VERTEX_CHECK_VEC3(storage, 0, color, 1.0f, 1.0f, 1.0f);
    VERTEX_CHECK_VEC2(storage, 0, texCoord, 0.0681650937f, 0.192196429f);
    VERTEX_CHECK_VEC4(storage, 0, tangent, -0.295275331f, -0.835973203f, -0.462559491f, 1.00000f);

    PRINT_VERTEX(storage, 1000);
    VERTEX_CHECK_VEC3(storage, 1000, pos, 0.00661812071f, -0.00235924753f, 0.00288167689f);
    VERTEX_CHECK_VEC3(storage, 1000, normal, 0.8339324f, 0.0814492553f, -0.545823038f);
    VERTEX_CHECK_VEC3(storage, 1000, color, 1.0f, 1.0f, 1.0f);
    VERTEX_CHECK_VEC2(storage, 1000, texCoord, 0.90863955f, 0.12404108f);
    VERTEX_CHECK_VEC4(storage, 1000, tangent, -0.114067227f, -0.993130505f, 0.0260791834f, 1.00000f);

    SECTION("CPUStorage merge")
    {
        AssetStorage::CPUStorage other_storage;
        REQUIRE_NOTHROW(other_storage = loaders::loadGltfModel(pathToOtherGltf).value());

        auto before = other_storage.modelStorage.at("0");
        auto before_vertex_size = other_storage.vertexStagingBuffer.size();
        auto before_index_size = other_storage.indexStagingBuffer.size();

        REQUIRE_NOTHROW(other_storage += storage);

        REQUIRE(other_storage.modelStorage.at("0") == before);
        REQUIRE_NOTHROW(other_storage.modelStorage.at("BoomBox"));
        REQUIRE(other_storage.modelStorage.at("BoomBox") == storage.modelStorage.at("BoomBox"));
        REQUIRE_NOTHROW(storage.modelStorage.at(testIds));

        {
            auto &other_model = other_storage.modelStorage.at(testIds);
            auto &model = storage.modelStorage.at(testIds);

            // REQUIRE(model.default_material == other_model.default_material);
            // REQUIRE(model.mesh.indicesOffset == other_model.mesh.indicesOffset - before_index_size);
            // REQUIRE(model.mesh.indicesSize == other_model.mesh.indicesSize);
            // REQUIRE(model.mesh.vertexOffset == other_model.mesh.vertexOffset - before_vertex_size);
            // REQUIRE(model.mesh.vertexSize == other_model.mesh.vertexSize);

            // std::span<Vertex> vertices(storage.vertexStagingBuffer.data() + model.mesh.vertexOffset,
            //                            model.mesh.vertexSize);
            // std::span<Vertex> other_vertices(other_storage.vertexStagingBuffer.data() +
            // other_model.mesh.vertexOffset,
            //                                  other_model.mesh.vertexSize);
            // REQUIRE(vertices.size() == other_vertices.size());
            // for (unsigned i = 0; i < vertices.size(); i++) REQUIRE(vertices[i] == other_vertices[i]);

            // std::span<std::uint32_t> indexes(storage.indexStagingBuffer.data() + model.mesh.indicesOffset,
            //                                  model.mesh.indicesSize);
            // std::span<std::uint32_t> other_indexes(
            //     other_storage.indexStagingBuffer.data() + other_model.mesh.indicesOffset,
            //     other_model.mesh.indicesSize);
            // REQUIRE(indexes.size() == other_indexes.size());
            // for (unsigned i = 0; i < indexes.size(); i++) REQUIRE(indexes[i] == other_indexes[i]);
        }

        REQUIRE_NOTHROW(other_storage.modelStorage.at("basic_triangle"));

        PRINT_VERTEX(other_storage, 3);
        VERTEX_CHECK_VEC3(other_storage, 3, pos, 0.00247455505f, -0.00207684329f, 0.00687769148f);
        VERTEX_CHECK_VEC3(other_storage, 3, normal, -0.937883496f, -0.345936537f, 0.0265043154f);
        VERTEX_CHECK_VEC3(other_storage, 3, color, 1.0f, 1.0f, 1.0f);
        VERTEX_CHECK_VEC2(other_storage, 3, texCoord, 0.0681650937f, 0.192196429f);
        VERTEX_CHECK_VEC4(other_storage, 3, tangent, -0.295275331f, -0.835973203f, -0.462559491f, 1.00000f);

        PRINT_VERTEX(other_storage, 1003);
        VERTEX_CHECK_VEC3(other_storage, 1003, pos, 0.00661812071f, -0.00235924753f, 0.00288167689f);
        VERTEX_CHECK_VEC3(other_storage, 1003, normal, 0.8339324f, 0.0814492553f, -0.545823038f);
        VERTEX_CHECK_VEC3(other_storage, 1003, color, 1.0f, 1.0f, 1.0f);
        VERTEX_CHECK_VEC2(other_storage, 1003, texCoord, 0.90863955f, 0.12404108f);
        VERTEX_CHECK_VEC4(other_storage, 1003, tangent, -0.114067227f, -0.993130505f, 0.0260791834f, 1.00000f);
    }
}

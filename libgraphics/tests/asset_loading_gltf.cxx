#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <span>

#include "pivot/graphics/AssetStorage/Loaders.hxx"
#include "pivot/graphics/types/Vertex.hxx"

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

    pivot::graphics::asset::CPUStorage storage;

    SECTION("Return false when incorrect path") REQUIRE_FALSE(asset::loaders::loadGltfModel(pathToObj).has_value());

    REQUIRE_NOTHROW(storage = asset::loaders::loadGltfModel(pathToGltf).value());

    std::string testIds = "BoomBox";
    REQUIRE_NOTHROW(storage.modelStorage.at("BoomBox"));
    REQUIRE(storage.modelStorage.size() == 2);

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

    REQUIRE(storage.vertexStagingBuffer.size() == 3575);
    REQUIRE(storage.indexStagingBuffer.size() == 18108);

    PRINT_VERTEX(storage, 0);
    VERTEX_CHECK_VEC3(storage, 0, pos, 0.00247455505f, 0.00207684329f, -0.00687769148f);
    VERTEX_CHECK_VEC3(storage, 0, normal, -0.937883496f, 0.345936537f, -0.0265043154f);
    VERTEX_CHECK_VEC3(storage, 0, color, 1.0f, 1.0f, 1.0f);
    VERTEX_CHECK_VEC2(storage, 0, texCoord, 0.0681650937f, 0.192196429f);
    VERTEX_CHECK_VEC4(storage, 0, tangent, -0.295275331f, -0.835973203f, -0.462559491f, 1.00000f);

    PRINT_VERTEX(storage, 1000);
    VERTEX_CHECK_VEC3(storage, 1000, pos, 0.00661812071f, 0.00235924753f, -0.00288167689f);
    VERTEX_CHECK_VEC3(storage, 1000, normal, 0.8339324f, -0.0814492553f, 0.545823038f);
    VERTEX_CHECK_VEC3(storage, 1000, color, 1.0f, 1.0f, 1.0f);
    VERTEX_CHECK_VEC2(storage, 1000, texCoord, 0.90863955f, 0.12404108f);
    VERTEX_CHECK_VEC4(storage, 1000, tangent, -0.114067227f, -0.993130505f, 0.0260791834f, 1.00000f);

    SECTION("CPUStorage merge")
    {
        asset::CPUStorage other_storage;
        REQUIRE_NOTHROW(other_storage = asset::loaders::loadGltfModel(pathToOtherGltf).value());

        auto before = other_storage.modelStorage.at("basic_triangle");
        auto before_vertex_size = other_storage.vertexStagingBuffer.size();
        auto before_index_size = other_storage.indexStagingBuffer.size();

        REQUIRE_NOTHROW(other_storage += storage);

        REQUIRE(other_storage.modelStorage.at("basic_triangle") == before);
        REQUIRE_NOTHROW(other_storage.modelStorage.at("BoomBox"));
        REQUIRE(other_storage.modelStorage.at("BoomBox") == storage.modelStorage.at("BoomBox"));
        REQUIRE_NOTHROW(storage.modelStorage.at(testIds));

        {
            auto &other_model = other_storage.modelStorage.at(testIds);
            auto &model = storage.modelStorage.at(testIds);

            REQUIRE(other_model->value.localMatrix == model->value.localMatrix);
            REQUIRE(other_model->value.name == model->value.name);
            REQUIRE(other_model->value.primitives.size() == model->value.primitives.size());
            for (unsigned i = 0; model->value.primitives.size(); i++) {
                REQUIRE(model->value.primitives.at(i).default_material ==
                        other_model->value.primitives.at(i).default_material);
                REQUIRE(model->value.primitives.at(i).indicesOffset ==
                        other_model->value.primitives.at(i).indicesOffset - before_index_size);
                REQUIRE(model->value.primitives.at(i).indicesSize == other_model->value.primitives.at(i).indicesSize);
                REQUIRE(model->value.primitives.at(i).vertexOffset ==
                        other_model->value.primitives.at(i).vertexOffset - before_vertex_size);
                REQUIRE(model->value.primitives.at(i).vertexSize == other_model->value.primitives.at(i).vertexSize);

                std::span<Vertex> vertices(storage.vertexStagingBuffer.data() +
                                               model->value.primitives.at(i).vertexOffset,
                                           model->value.primitives.at(i).vertexSize);
                std::span<Vertex> other_vertices(other_storage.vertexStagingBuffer.data() +
                                                     other_model->value.primitives.at(i).vertexOffset,
                                                 other_model->value.primitives.at(i).vertexSize);
                REQUIRE(vertices.size() == other_vertices.size());
                for (unsigned i = 0; i < vertices.size(); i++) REQUIRE(vertices[i] == other_vertices[i]);

                std::span<std::uint32_t> indexes(storage.indexStagingBuffer.data() +
                                                     model->value.primitives.at(i).indicesOffset,
                                                 model->value.primitives.at(i).indicesSize);
                std::span<std::uint32_t> other_indexes(other_storage.indexStagingBuffer.data() +
                                                           other_model->value.primitives.at(i).indicesOffset,
                                                       other_model->value.primitives.at(i).indicesSize);
                REQUIRE(indexes.size() == other_indexes.size());
                for (unsigned i = 0; i < indexes.size(); i++) REQUIRE(indexes[i] == other_indexes[i]);
            }
        }

        REQUIRE_NOTHROW(other_storage.modelStorage.at("basic_triangle"));

        VERTEX_CHECK_VEC3(storage, 0, pos, 0.00247455505f, 0.00207684329f, -0.00687769148f);
        VERTEX_CHECK_VEC3(storage, 0, normal, -0.937883496f, 0.345936537f, -0.0265043154f);
        VERTEX_CHECK_VEC3(storage, 0, color, 1.0f, 1.0f, 1.0f);
        VERTEX_CHECK_VEC2(storage, 0, texCoord, 0.0681650937f, 0.192196429f);
        VERTEX_CHECK_VEC4(storage, 0, tangent, -0.295275331f, -0.835973203f, -0.462559491f, 1.00000f);

        VERTEX_CHECK_VEC3(storage, 1000, pos, 0.00661812071f, 0.00235924753f, -0.00288167689f);
        VERTEX_CHECK_VEC3(storage, 1000, normal, 0.8339324f, -0.0814492553f, 0.545823038f);
        VERTEX_CHECK_VEC3(storage, 1000, color, 1.0f, 1.0f, 1.0f);
        VERTEX_CHECK_VEC2(storage, 1000, texCoord, 0.90863955f, 0.12404108f);
        VERTEX_CHECK_VEC4(storage, 1000, tangent, -0.114067227f, -0.993130505f, 0.0260791834f, 1.00000f);
    }
}

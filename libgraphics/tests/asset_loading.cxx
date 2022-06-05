#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/VulkanApplication.hxx"

constexpr const std::string_view miniGltf =
    "{"
    "  \"scene\": 0,"
    "  \"scenes\" : ["
    "    {"
    "      \"nodes\" : [ 0 ]"
    "    }"
    "  ],"
    "  "
    "  \"nodes\" : ["
    "    {"
    "      \"name\" :  \"triangle\","
    "      \"mesh\" : 0"
    "    }"
    "  ],"
    "  "
    "  \"meshes\" : ["
    "    {"
    "      \"primitives\" : [ {"
    "        \"attributes\" : {"
    "          \"POSITION\" : 1"
    "        },"
    "        \"indices\" : 0"
    "      } ]"
    "    }"
    "  ],"
    ""
    "  \"buffers\" : ["
    "    {"
    "      \"uri\" : "
    "\"data:application/octet-stream;base64,AAABAAIAAAAAAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAAAAAACAPwAAAAA=\","
    "      \"byteLength\" : 44"
    "    }"
    "  ],"
    "  \"bufferViews\" : ["
    "    {"
    "      \"buffer\" : 0,"
    "      \"byteOffset\" : 0,"
    "      \"byteLength\" : 6,"
    "      \"target\" : 34963"
    "    },"
    "    {"
    "      \"buffer\" : 0,"
    "      \"byteOffset\" : 8,"
    "      \"byteLength\" : 36,"
    "      \"target\" : 34962"
    "    }"
    "  ],"
    "  \"accessors\" : ["
    "    {"
    "      \"bufferView\" : 0,"
    "      \"byteOffset\" : 0,"
    "      \"componentType\" : 5123,"
    "      \"count\" : 3,"
    "      \"type\" : \"SCALAR\","
    "      \"max\" : [ 2 ],"
    "      \"min\" : [ 0 ]"
    "    },"
    "    {"
    "      \"bufferView\" : 1,"
    "      \"byteOffset\" : 0,"
    "      \"componentType\" : 5126,"
    "      \"count\" : 3,"
    "      \"type\" : \"VEC3\","
    "      \"max\" : [ 1.0, 1.0, 0.0 ],"
    "      \"min\" : [ 0.0, 0.0, 0.0 ]"
    "    }"
    "  ],"
    "  "
    "  \"asset\" :"
    "{"
    "    \"version\" : \"2.0\""
    "}"
    "}";

constexpr const std::string_view objFileContent = "o Plane\n"
                                                  "g square\n"
                                                  "v -50.000000 0.000000 50.000000 0.1 0 0.2\n"
                                                  "v 50.000000 0.000000 50.000000 1 0.2 0.3\n"
                                                  "v 50.000000 0.000000 -50.000000 0 0.5 1\n"
                                                  "v -50.000000 0.000000 -50.000000 0.1 0 1\n"
                                                  "vt -10.000000 10.000000\n"
                                                  "vt 10.000000 10.000000\n"
                                                  "vt 10.000000 -10.000000\n"
                                                  "vt -10.000000 -10.000000\n"
                                                  "vn 0.0000 1.0000 0.0000\n"
                                                  "s 1\n"
                                                  "f 1/1/1 2/2/1 3/3/1 4/4/1\n";

const std::string pathToGltf = "mini.gltf";
const std::string pathToObj = "mini.obj";

void createFile(const std::string &path, const std::string_view &content)
{
    std::ofstream myfile;
    myfile.open(path);
    myfile.write(content.data(), content.size());
    myfile.close();
}

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
    createFile(pathToGltf, miniGltf);
    AssetStorage::CPUStorage storage;

    // SECTION("Return false when incorrect path") REQUIRE_FALSE(loaders::loadGltfModel(pathToObj, storage));

    REQUIRE(loaders::loadGltfModel(pathToGltf, storage));

    REQUIRE_NOTHROW(storage.prefabStorage.at("mini"));
    const auto &prefab = storage.prefabStorage.at("mini");
    REQUIRE(prefab.modelIds.size() == 1);

    const auto offset = AssetStorage::quad_vertices.size();

    REQUIRE_NOTHROW(storage.modelStorage.at("triangle" + std::to_string(offset)));
    const auto &model = storage.modelStorage.at("triangle" + std::to_string(offset));
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

    std::filesystem::remove(pathToGltf);
}

TEST_CASE("loadObjFile", "[assetStorage]")
{
    createFile(pathToObj, objFileContent);

    AssetStorage::CPUStorage storage;
    // SECTION("Return false when incorrect path") { REQUIRE_FALSE(loaders::loadObjModel(pathToGltf, storage)); }

    REQUIRE(loaders::loadObjModel(pathToObj, storage));

    const auto &prefab = storage.prefabStorage.at("mini");
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

    std::filesystem::remove(pathToObj);
}

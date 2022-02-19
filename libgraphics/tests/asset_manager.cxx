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
    myfile << content;
    myfile.close();
}

using namespace pivot::graphics;

TEST_CASE("loadGltfFile", "[assetStorage]")
{
    createFile(pathToGltf, miniGltf);

    VulkanApplication app;
    REQUIRE_NOTHROW(app.assetStorage.loadModels(pathToGltf));

    const auto &[vertexStorage, indexStorage, textureStorage, materialStorage] = app.assetStorage.getCPUStorage();
    REQUIRE(textureStorage.empty());
    REQUIRE(materialStorage.size() == 1);
    REQUIRE_NOTHROW(materialStorage.get("white"));
    const auto &prefab = app.assetStorage.get<AssetStorage::Prefab>("mini");
    REQUIRE(prefab.modelIds.size() == 1);

    const auto &model = app.assetStorage.get<AssetStorage::Model>("triangle0");
    REQUIRE(model.default_material.has_value());
    REQUIRE(model.default_material == "white");
    REQUIRE(model.mesh.vertexOffset == 0);
    REQUIRE(model.mesh.vertexSize == 3);
    REQUIRE(model.mesh.indicesOffset == 0);
    REQUIRE(model.mesh.indicesSize == 3);

    REQUIRE(vertexStorage.at(0).pos.x == 0.0f);
    REQUIRE(vertexStorage.at(0).pos.y == 0.0f);
    REQUIRE(vertexStorage.at(0).pos.z == 0.0f);
    REQUIRE(vertexStorage.at(0).normal == glm::vec3(0.0f));
    REQUIRE(vertexStorage.at(0).color == glm::vec3(1.0f));
    REQUIRE(vertexStorage.at(0).texCoord == glm::vec2(0.0f));

    REQUIRE(vertexStorage.at(1).pos.x == 1.0f);
    REQUIRE(vertexStorage.at(1).pos.y == 0.0f);
    REQUIRE(vertexStorage.at(1).pos.z == 0.0f);
    REQUIRE(vertexStorage.at(1).normal == glm::vec3(0.0f));
    REQUIRE(vertexStorage.at(1).color == glm::vec3(1.0f));
    REQUIRE(vertexStorage.at(1).texCoord == glm::vec2(0.0f));

    REQUIRE(vertexStorage.at(2).pos.x == 0.0f);
    REQUIRE(vertexStorage.at(2).pos.y == 1.0f);
    REQUIRE(vertexStorage.at(2).pos.z == 0.0f);
    REQUIRE(vertexStorage.at(2).normal == glm::vec3(0.0f));
    REQUIRE(vertexStorage.at(2).color == glm::vec3(1.0f));
    REQUIRE(vertexStorage.at(2).texCoord == glm::vec2(0.0f));

    std::filesystem::remove(pathToGltf);
}

TEST_CASE("loadObjFile", "[assetStorage]")
{
    createFile(pathToObj, objFileContent);

    VulkanApplication app;
    REQUIRE_NOTHROW(app.assetStorage.loadModels(pathToObj));

    const auto &[vertexStorage, indexStorage, textureStorage, materialStorage] = app.assetStorage.getCPUStorage();
    REQUIRE(textureStorage.empty());
    REQUIRE(materialStorage.size() == 1);
    REQUIRE_NOTHROW(materialStorage.get("white"));
    const auto &prefab = app.assetStorage.get<AssetStorage::Prefab>("mini");
    REQUIRE(prefab.modelIds.size() == 1);

    const auto &model = app.assetStorage.get<AssetStorage::Model>("square");
    REQUIRE(model.default_material.has_value());
    REQUIRE(model.default_material == "white");
    REQUIRE(model.mesh.vertexOffset == 0);
    REQUIRE(model.mesh.vertexSize == 4);
    REQUIRE(model.mesh.indicesOffset == 0);
    REQUIRE(model.mesh.indicesSize == 4);

    REQUIRE(vertexStorage.at(0).pos.x == -50.0f);
    REQUIRE(vertexStorage.at(0).pos.y == 0.0f);
    REQUIRE(vertexStorage.at(0).pos.z == 50.0f);
    REQUIRE(vertexStorage.at(0).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(vertexStorage.at(0).color.x == 0.1f);
    REQUIRE(vertexStorage.at(0).color.y == 0.0f);
    REQUIRE(vertexStorage.at(0).color.z == 0.2f);
    REQUIRE(vertexStorage.at(0).texCoord.x == -10.f);
    REQUIRE(vertexStorage.at(0).texCoord.y == 10.f);

    REQUIRE(vertexStorage.at(1).pos.x == 50.0f);
    REQUIRE(vertexStorage.at(1).pos.y == 0.0f);
    REQUIRE(vertexStorage.at(1).pos.z == 50.0f);
    REQUIRE(vertexStorage.at(1).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(vertexStorage.at(1).color.x == 1.f);
    REQUIRE(vertexStorage.at(1).color.y == 0.2f);
    REQUIRE(vertexStorage.at(1).color.z == 0.3f);
    REQUIRE(vertexStorage.at(1).texCoord.x == 10.f);
    REQUIRE(vertexStorage.at(1).texCoord.y == 10.f);

    REQUIRE(vertexStorage.at(2).pos.x == 50.0f);
    REQUIRE(vertexStorage.at(2).pos.y == 0.0f);
    REQUIRE(vertexStorage.at(2).pos.z == -50.0f);
    REQUIRE(vertexStorage.at(2).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(vertexStorage.at(2).color.x == 0.0f);
    REQUIRE(vertexStorage.at(2).color.y == 0.5f);
    REQUIRE(vertexStorage.at(2).color.z == 1.0f);
    REQUIRE(vertexStorage.at(2).texCoord.x == 10.f);
    REQUIRE(vertexStorage.at(2).texCoord.y == -10.f);

    REQUIRE(vertexStorage.at(3).pos.x == -50.0f);
    REQUIRE(vertexStorage.at(3).pos.y == 0.0f);
    REQUIRE(vertexStorage.at(3).pos.z == -50.0f);
    REQUIRE(vertexStorage.at(3).normal == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(vertexStorage.at(3).color.x == 0.1f);
    REQUIRE(vertexStorage.at(3).color.y == 0.0f);
    REQUIRE(vertexStorage.at(3).color.z == 1.0f);
    REQUIRE(vertexStorage.at(3).texCoord.x == -10.f);
    REQUIRE(vertexStorage.at(3).texCoord.y == -10.f);

    std::filesystem::remove(pathToObj);
}

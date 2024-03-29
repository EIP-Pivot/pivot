#include "pivot/graphics/AssetStorage/Loaders.hxx"

#include "pivot/pivot.hxx"

#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader.h>

namespace pivot::graphics::loaders
{

static std::pair<std::string, asset::CPUMaterial> loadMaterial(const tinyobj::material_t &material)
{
    std::filesystem::path diffuse = material.diffuse_texname;
    std::filesystem::path normal = material.normal_texname;
    std::filesystem::path emissive = material.emissive_texname;
    std::filesystem::path specular = material.specular_texname;
    std::filesystem::path specular_highlight = material.specular_highlight_texname;

    return {
        material.name,
        asset::CPUMaterial{
            .alphaCutOff = 1.0f,
            .metallicFactor = material.metallic,
            .roughnessFactor = material.roughness,
            .baseColor =
                {
                    material.diffuse[0],
                    material.diffuse[1],
                    material.diffuse[2],
                    1.0f,
                },
            .baseColorFactor =
                {
                    material.diffuse_texopt.scale[0],
                    material.diffuse_texopt.scale[1],
                    material.diffuse_texopt.scale[2],
                    1.0f,
                },
            .emissiveFactor =
                {
                    material.emissive_texopt.scale[0],
                    material.emissive_texopt.scale[1],
                    material.emissive_texopt.scale[2],
                    1.0f,
                },
            .baseColorTexture = diffuse.stem().string(),
            .normalTexture = normal.stem().string(),
            .emissiveTexture = emissive.stem().string(),
            .specularGlossinessTexture = specular_highlight.stem().string(),
            .diffuseTexture = diffuse.stem().string(),
        },
    };
}

std::optional<asset::CPUStorage> loadObjModel(const std::filesystem::path &path)
{
    DEBUG_FUNCTION();
    auto base_dir = path.parent_path();

    asset::CPUStorage storage;
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool loadSuccess = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(),
                                        base_dir.string().c_str(), false, false);
    if (!warn.empty()) logger.warn("Asset Storage/OBJ") << warn;
    if (!err.empty()) logger.err("Asset Storage/OBJ") << err;
    if (!loadSuccess) return std::nullopt;

    if (shapes.empty()) {
        logger.warn("Asset Storage/OBJ") << "No shapes was loaded, this is considered as a failure.";
        return std::nullopt;
    }

    for (const auto &m: materials) {
        if (!m.diffuse_texname.empty() && storage.textureStaging.getIndex(m.diffuse_texname) == -1) {
            const auto filepath = base_dir / m.diffuse_texname;
            storage.texturePaths.emplace(filepath.stem().string(), filepath);
        }
        storage.materialStaging.add(loadMaterial(m));
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    asset::Prefab prefab;
    for (const auto &shape: shapes) {
        asset::Model model{
            .mesh =
                {
                    .vertexOffset = static_cast<uint32_t>(storage.vertexStagingBuffer.size()),
                    .vertexSize = 0,
                    .indicesOffset = static_cast<uint32_t>(storage.indexStagingBuffer.size()),
                    .indicesSize = 0,
                },
            .default_material = ((!shape.mesh.material_ids.empty() && shape.mesh.material_ids.at(0) >= 0)
                                     ? (materials.at(shape.mesh.material_ids.at(0)).name)
                                     : ("white")),
        };
        for (const auto &index: shape.mesh.indices) {
            pivotAssertMsg(index.vertex_index != -1, "No vertices in the obj file");
            Vertex vertex{
                .pos =
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    },
            };
            if (!attrib.colors.empty()) {
                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };
            }
            if (!attrib.normals.empty() && index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }
            if (!attrib.texcoords.empty() && index.texcoord_index >= 0) {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }

            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices.emplace(vertex, storage.vertexStagingBuffer.size() - model.mesh.vertexOffset);
                storage.vertexStagingBuffer.push_back(vertex);
            }
            storage.indexStagingBuffer.push_back(uniqueVertices.at(vertex));
        }
        model.mesh.indicesSize = storage.indexStagingBuffer.size() - model.mesh.indicesOffset;
        model.mesh.vertexSize = storage.vertexStagingBuffer.size() - model.mesh.vertexOffset;
        prefab.modelIds.push_back(shape.name + std::to_string(model.mesh.vertexOffset));
        storage.modelStorage.emplace(shape.name + std::to_string(model.mesh.vertexOffset), model);
    }
    storage.prefabStorage.emplace(path.stem().string(), prefab);
    return storage;
}

}    // namespace pivot::graphics::loaders

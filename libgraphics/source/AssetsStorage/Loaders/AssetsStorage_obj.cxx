#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader.h>

namespace pivot::graphics
{

static std::pair<std::string, AssetStorage::CPUMaterial> loadMaterial(const tinyobj::material_t &material)
{
    std::filesystem::path diffuse = material.diffuse_texname;
    AssetStorage::CPUMaterial materia{
        .metallic = material.metallic,
        .roughness = material.roughness,
        .baseColor =
            {
                material.diffuse[0],
                material.diffuse[1],
                material.diffuse[2],
                0.0f,
            },
        .baseColorTexture = diffuse.stem().string(),
    };
    return std::make_pair(material.name, std::move(materia));
}

bool AssetStorage::loadObjModel(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    auto base_dir = path.parent_path();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool loadSuccess = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(),
                                        base_dir.string().c_str(), false, false);
    if (!warn.empty()) logger.warn("Asset Storage/OBJ") << warn;
    if (!err.empty()) logger.err("Asset Storage/OBJ") << err;
    if (!loadSuccess) return false;

    for (const auto &m: materials) {
        if (!m.diffuse_texname.empty() && cpuStorage.textureStaging.getIndex(m.diffuse_texname) == -1) {
            loadTexture(base_dir / m.diffuse_texname);
        }
        cpuStorage.materialStaging.add(loadMaterial(m));
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    Prefab prefab;
    for (const auto &shape: shapes) {
        Model model{
            .mesh =
                {
                    .vertexOffset = static_cast<uint32_t>(cpuStorage.vertexStagingBuffer.size()),
                    .indicesOffset = static_cast<uint32_t>(cpuStorage.indexStagingBuffer.size()),
                },
        };
        if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids.at(0) >= 0) {
            model.default_material = materials.at(shape.mesh.material_ids.at(0)).name;
        }
        for (const auto &index: shape.mesh.indices) {
            assert(index.vertex_index >= 0);
            Vertex vertex{
                .pos =
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    },
                .color = {1.0f, 1.0f, 1.0f},
            };
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
            if (!attrib.colors.empty()) {
                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };
            }

            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices.insert(
                    std::make_pair(vertex, cpuStorage.vertexStagingBuffer.size() - model.mesh.vertexOffset));
                cpuStorage.vertexStagingBuffer.push_back(vertex);
            }
            cpuStorage.indexStagingBuffer.push_back(uniqueVertices.at(vertex));
        }
        model.mesh.indicesSize = cpuStorage.indexStagingBuffer.size() - model.mesh.indicesOffset;
        model.mesh.vertexSize = cpuStorage.vertexStagingBuffer.size() - model.mesh.vertexOffset;
        prefab.modelIds.push_back(shape.name);
        modelStorage.insert({shape.name, model});
        meshBoundingBoxStorage.add(
            shape.name, MeshBoundingBox(std::span(cpuStorage.vertexStagingBuffer.begin() + model.mesh.vertexOffset,
                                                  model.mesh.vertexSize)));
    }
    prefabStorage.insert({path.stem().string(), prefab});
    return true;
}

}    // namespace pivot::graphics
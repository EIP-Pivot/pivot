#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <glm/gtc/type_ptr.hpp>

namespace pivot::graphics
{

bool AssetStorage::loadObjModel(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    auto base_dir = path.parent_path();
    std::vector<Vertex> currentVertexBuffer;
    std::vector<uint32_t> currentIndexBuffer;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    /// TODO: check return value
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), base_dir.string().c_str(), false,
                     false);
    if (!warn.empty()) { logger.warn("LOADING_OBJ") << warn; }
    if (!err.empty()) {
        logger.err("LOADING_OBJ") << err;
        return false;
    }

    for (const auto &m: materials) {
        loadMaterial(m);
        if (!m.diffuse_texname.empty() && !getTextures().contains(m.diffuse_texname)) {
            loadTexture(base_dir / m.diffuse_texname);
        }
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    Prefab prefab;
    for (const auto &shape: shapes) {
        Model model{
            .mesh =
                {
                    .vertexOffset = static_cast<uint32_t>(vertexStagingBuffer.size()),
                    .indicesOffset = static_cast<uint32_t>(indexStagingBuffer.size()),
                },
        };
        if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids.at(0) >= 0) {
            model.default_material = materials.at(shape.mesh.material_ids.at(0)).name;
            if (std::filesystem::path name = materials.at(shape.mesh.material_ids.at(0)).diffuse_texname;
                !name.empty()) {
                model.default_texture = name.stem().string();
            } else if (!prefab.modelIds.empty()) {
                model.default_texture = get<Model>(prefab.modelIds.at(0)).default_texture;
            }
        }
        for (const auto &index: shape.mesh.indices) {
            Vertex vertex{
                .pos =
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    },
                .color = {1.0f, 1.0f, 1.0f},
            };
            if (!attrib.normals.empty()) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }
            if (!attrib.texcoords.empty()) {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }

            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = vertexStagingBuffer.size() - model.mesh.vertexOffset;
                vertexStagingBuffer.push_back(vertex);
            }
            indexStagingBuffer.push_back(uniqueVertices.at(vertex));
        }
        model.mesh.indicesSize = indexStagingBuffer.size() - model.mesh.indicesOffset;
        model.mesh.vertexSize = vertexStagingBuffer.size() - model.mesh.vertexOffset;
        prefab.modelIds.push_back(shape.name);
        modelStorage.insert({shape.name, model});
        meshBoundingBoxStorage.insert(
            {shape.name,
             MeshBoundingBox(std::span(vertexStagingBuffer.begin() + model.mesh.vertexOffset, model.mesh.vertexSize))});
    }
    prefabStorage.insert({path.stem().string(), prefab});
    vertexStagingBuffer.insert(vertexStagingBuffer.end(), currentVertexBuffer.begin(), currentVertexBuffer.end());
    indexStagingBuffer.insert(indexStagingBuffer.end(), currentIndexBuffer.begin(), currentIndexBuffer.end());
    return true;
}

bool AssetStorage::loadMaterial(const tinyobj::material_t &material)
{
    gpuObject::Material mat{
        .shininess = material.shininess,
        .ambientColor = glm::make_vec3(material.ambient),
        .diffuse = glm::make_vec3(material.diffuse),
        .specular = glm::make_vec3(material.specular),
    };
    materialStorage.insert({material.name, mat});
    return true;
}

}    // namespace pivot::graphics
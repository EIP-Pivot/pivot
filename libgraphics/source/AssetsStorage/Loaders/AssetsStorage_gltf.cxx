#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ktx.h>
#include <ktxvulkan.h>
#include <tiny_gltf.h>

static bool loadImageDataFunc(tinygltf::Image *image, const int imageIndex, std::string *error, std::string *warning,
                              int req_width, int req_height, const unsigned char *bytes, int size, void *userData)
{
    if (image->uri.find_last_of(".") != std::string::npos) {
        if (image->uri.substr(image->uri.find_last_of(".") + 1) == "ktx") { return true; }
    }

    return tinygltf::LoadImageData(image, imageIndex, error, warning, req_width, req_height, bytes, size, userData);
}

static std::pair<const float *const, const tinygltf::Accessor &>
getGltfBuffer(const tinygltf::Model &model, const tinygltf::Primitive &primitive, const std::string &name)
{
    auto iter = primitive.attributes.find(name);
    if (iter != primitive.attributes.end()) {
        const tinygltf::Accessor &accessor = model.accessors.at(iter->second);
        const tinygltf::BufferView &view = model.bufferViews.at(accessor.bufferView);
        assert(view.byteStride == 0);
        return {reinterpret_cast<const float *>(
                    &(model.buffers.at(view.buffer).data.at(accessor.byteOffset + view.byteOffset))),
                accessor};
    }
    return {nullptr, {}};
}

namespace pivot::graphics
{

static std::vector<std::pair<std::string, AssetStorage::Model>> loadGltfNode(const tinygltf::Model &gltfModel,
                                                                             const tinygltf::Node &node,
                                                                             std::vector<Vertex> &vertexBuffer,
                                                                             std::vector<uint32_t> &indexBuffer)
{
    logger.info("AssetStorage/Gltf") << "Loading node: " << node.name;
    std::vector<std::pair<std::string, AssetStorage::Model>> loaded;
    for (const auto &i: node.children) {
        auto child = loadGltfNode(gltfModel, gltfModel.nodes.at(i), vertexBuffer, indexBuffer);
        loaded.insert(loaded.end(), child.begin(), child.end());
    }
    if (node.mesh <= -1) return {};
    const auto &mesh = gltfModel.meshes.at(node.mesh);
    AssetStorage::Model model{
        .mesh =
            {
                .vertexOffset = static_cast<uint32_t>(vertexBuffer.size()),
                .indicesOffset = static_cast<uint32_t>(indexBuffer.size()),
            },
    };
    // glm::mat4 matrix;
    // if (node.translation.size() == 3) {
    //     matrix = glm::translate(matrix, glm::vec3(glm::make_vec3(node.translation.data())));
    // }
    // if (node.rotation.size() == 4) {
    //     glm::quat q = glm::make_quat(node.rotation.data());
    //     matrix *= glm::mat4(q);
    // }
    // if (node.scale.size() == 3) { matrix = glm::scale(matrix, glm::vec3(glm::make_vec3(node.scale.data()))); }
    // if (node.matrix.size() == 16) { matrix = glm::make_mat4x4(node.matrix.data()); }

    for (const tinygltf::Primitive &primitive: mesh.primitives) {
        // Vertices
        {
            const auto &[positionBuffer, positionAccessor] = getGltfBuffer(gltfModel, primitive, "POSITION");
            const auto &[normalsBuffer, normalAccessor] = getGltfBuffer(gltfModel, primitive, "NORMAL");
            const auto &[texCoordsBuffer, texCoordsAccessor] = getGltfBuffer(gltfModel, primitive, "TEXCOORD_0");
            const auto &[colorBuffer, colorAccessor] = getGltfBuffer(gltfModel, primitive, "TEXCOORD_0");
            auto numColorComponents = colorAccessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;

            for (std::size_t i = 0; i < positionAccessor.count; i++) {
                logger.debug() << i << "(" << positionBuffer[i * 3] << ", " << positionBuffer[i * 3 + 1] << ", "
                               << positionBuffer[i * 3 + 2] << ")";
                if (std::isnan(positionBuffer[i * 3]) || std::isnan(positionBuffer[i * 3 + 1]),
                    std::isnan(positionBuffer[i * 3 + 2])) {
                    abort();
                }
            }

            assert(positionBuffer);
            model.mesh.vertexSize = positionAccessor.count;
            for (std::size_t v = 0; v < positionAccessor.count; v++) {
                Vertex vert{};
                vert.pos = glm::make_vec3(&positionBuffer[v * 3]);
                vert.normal = normalsBuffer ? (glm::normalize(glm::make_vec3(&normalsBuffer[v * 3]))) : glm::vec3(0.0f);
                vert.texCoord = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                if (colorBuffer) {
                    switch (numColorComponents) {
                        case 3: vert.color = glm::vec4(glm::make_vec3(&colorBuffer[v * 3]), 1.0f); break;
                        case 4: vert.color = glm::make_vec4(&colorBuffer[v * 4]); break;
                        default:
                            throw AssetStorage::AssetStorageException(std::to_string(numColorComponents) +
                                                                      " colors components is not supported");
                    }
                } else {
                    vert.color = glm::vec4(1.0f);
                }
                vertexBuffer.push_back(vert);
            }
        }
        /// End of Verticies
        /// Indices
        {
            const tinygltf::Accessor &accessor = gltfModel.accessors[primitive.indices];
            const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

            model.mesh.indicesSize += static_cast<uint32_t>(accessor.count);

            // glTF supports different component types of indices
            switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    const uint32_t *buf = reinterpret_cast<const uint32_t *>(
                        &buffer.data.at(accessor.byteOffset + bufferView.byteOffset));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + model.mesh.vertexOffset);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const uint16_t *buf = reinterpret_cast<const uint16_t *>(
                        &buffer.data.at(accessor.byteOffset + bufferView.byteOffset));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + model.mesh.vertexOffset);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const uint8_t *buf =
                        reinterpret_cast<const uint8_t *>(&buffer.data.at(accessor.byteOffset + bufferView.byteOffset));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + model.mesh.vertexOffset);
                    }
                    break;
                }
                default: throw AssetStorage::AssetStorageException("Index component type not supported!"); break;
            }
        }
        /// End of Indices
        if (primitive.material > -1)
            model.default_material = gltfModel.materials.at(primitive.material).name;
        else
            model.default_material = "white";
    }
    loaded.push_back(std::make_pair(node.name, model));
    return loaded;
}

static std::pair<std::string, AssetStorage::CPUMaterial> loadGltfMaterial(const tinygltf::Model &gltfModel,
                                                                          const tinygltf::Material &mat)
{
    AssetStorage::CPUMaterial material;
    if (mat.values.find("baseColorTexture") != mat.values.end()) {
        material.baseColorTexture = gltfModel.textures.at(mat.values.at("baseColorTexture").TextureIndex()).name;
    }
    // Metallic roughness workflow
    if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
        material.metallicRoughnessTexture =
            gltfModel.textures.at(mat.values.at("metallicRoughnessTexture").TextureIndex()).name;
    }
    if (mat.values.find("roughnessFactor") != mat.values.end()) {
        material.roughness = mat.values.at("roughnessFactor").Factor();
    }
    if (mat.values.find("metallicFactor") != mat.values.end()) {
        material.metallic = mat.values.at("metallicFactor").Factor();
    }
    if (mat.values.find("baseColorFactor") != mat.values.end()) {
        material.baseColor = glm::make_vec4(mat.values.at("baseColorFactor").ColorFactor().data());
    }
    if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
        material.normalTexture = gltfModel.textures[mat.additionalValues.at("normalTexture").TextureIndex()].name;
    }
    if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
        material.emissiveTexture =
            gltfModel.textures.at(mat.additionalValues.at("emissiveTexture").TextureIndex()).name;
    }
    if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
        material.occlusionTexture =
            gltfModel.textures.at(mat.additionalValues.at("occlusionTexture").TextureIndex()).name;
    }
    return std::make_pair(mat.name, material);
}

static std::pair<std::string, AssetStorage::CPUTexture> loadGltfTexture(const tinygltf::Image &gltfimage,
                                                                        const std::filesystem::path &base_path)
{
    AssetStorage::CPUTexture texture;
    auto path = base_path / gltfimage.uri;
    if (path.extension() == ".ktx") {
        ktxTexture *ktxTexture = nullptr;
        auto result =
            ktxTexture_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
        if (result != KTX_SUCCESS) throw AssetStorage::AssetStorageException("Failed to load ktx texture");
        ktx_uint8_t *ktxTextureData = ktxTexture_GetData(ktxTexture);
        ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);
        texture.image.resize(ktxTextureSize);
        std::memcpy(texture.image.data(), ktxTextureData, ktxTextureSize);
        texture.size = vk::Extent3D{
            .width = ktxTexture->baseWidth,
            .height = ktxTexture->baseHeight,
            .depth = ktxTexture->baseDepth,
        };
        ktxTexture_Destroy(ktxTexture);
    } else {
        auto size = gltfimage.width * gltfimage.height * 4;
        texture.image.resize(size);
        std::memcpy(texture.image.data(), gltfimage.image.data(), size);
        texture.size = vk::Extent3D{
            .width = static_cast<uint32_t>(gltfimage.width),
            .height = static_cast<uint32_t>(gltfimage.height),
            .depth = 1,
        };
    }
    return {gltfimage.name, texture};
}

bool AssetStorage::loadGltfModel(const std::filesystem::path &path)
{
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    gltfContext.SetImageLoader(loadImageDataFunc, nullptr);
    std::string error, warning;
    Prefab prefab;

    gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, path.string());
    if (!warning.empty()) { logger.warn("Asset Storage/GLTF") << warning; }
    if (!error.empty()) {
        logger.err("Asset Storage/GLTF") << error;
        return false;
    }
    try {
        for (const auto &image: gltfModel.images) {
            logger.info("Asset Storage/Texture") << "Loaded texture: " << image.name;
            cpuStorage.textureStaging.add(loadGltfTexture(image, path.parent_path()));
        }
        for (const auto &material: gltfModel.materials) {
            auto mat = loadGltfMaterial(gltfModel, material);
            cpuStorage.materialStaging.add(mat);
        }
        for (const auto &node: gltfModel.nodes) {
            for (const auto &i:
                 loadGltfNode(gltfModel, node, cpuStorage.vertexStagingBuffer, cpuStorage.indexStagingBuffer)) {
                modelStorage.insert(i);
                prefab.modelIds.push_back(i.first);
                meshBoundingBoxStorage.add(i.first, MeshBoundingBox(std::span(cpuStorage.vertexStagingBuffer.begin() +
                                                                                  i.second.mesh.vertexOffset,
                                                                              i.second.mesh.vertexSize)));
            }
        }
    } catch (const AssetStorageException &ase) {
        logger.err("THROW/Asset Storage/GLTF") << "Error while loaded GLTF file : " << ase.what();
        return false;
    }
    prefabStorage.insert(std::make_pair(path.stem().string(), prefab));
    return true;
}

}    // namespace pivot::graphics
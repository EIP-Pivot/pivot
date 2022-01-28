#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <glm/gtc/type_ptr.hpp>
#include <ktx.h>
#include <ktxvulkan.h>
#include <tiny_gltf.h>

static bool loadImageDataFunc(tinygltf::Image *image, const int imageIndex, std::string *error, std::string *warning,
                              int req_width, int req_height, const unsigned char *bytes, int size, void *userData)
{
    // KTX files will be handled by our own code
    if (image->uri.find_last_of(".") != std::string::npos) {
        if (image->uri.substr(image->uri.find_last_of(".") + 1) == "ktx") { return true; }
    }

    return tinygltf::LoadImageData(image, imageIndex, error, warning, req_width, req_height, bytes, size, userData);
}

namespace pivot::graphics
{

static std::vector<std::pair<std::string, AssetStorage::Model>> loadGltfNode(const tinygltf::Model &gltfModel,
                                                                             const tinygltf::Node &node,
                                                                             std::vector<Vertex> &vertexBuffer,
                                                                             std::vector<uint32_t> &indexBuffer)
{
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
    for (const tinygltf::Primitive &primitive: mesh.primitives) {
        if (primitive.indices < 0) continue;
        // Vertices
        {
            const float *bufferPos = nullptr;
            const float *bufferNormals = nullptr;
            const float *bufferTexCoords = nullptr;
            const float *bufferColors = nullptr;
            uint32_t numColorComponents = 0;

            assert(primitive.attributes.find("POSITION") != primitive.attributes.end());
            const tinygltf::Accessor &posAccessor = gltfModel.accessors[primitive.attributes.find("POSITION")->second];
            const tinygltf::BufferView &posView = gltfModel.bufferViews[posAccessor.bufferView];
            bufferPos = reinterpret_cast<const float *>(
                &(gltfModel.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                const tinygltf::Accessor &normAccessor =
                    gltfModel.accessors[primitive.attributes.find("NORMAL")->second];
                const tinygltf::BufferView &normView = gltfModel.bufferViews[normAccessor.bufferView];
                bufferNormals = reinterpret_cast<const float *>(
                    &(gltfModel.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
            }

            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                const tinygltf::Accessor &uvAccessor =
                    gltfModel.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView &uvView = gltfModel.bufferViews[uvAccessor.bufferView];
                bufferTexCoords = reinterpret_cast<const float *>(
                    &(gltfModel.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
            }
            if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
                const tinygltf::Accessor &colorAccessor =
                    gltfModel.accessors[primitive.attributes.find("COLOR_0")->second];
                const tinygltf::BufferView &colorView = gltfModel.bufferViews[colorAccessor.bufferView];
                // Color buffer are either of type vec3 or vec4
                numColorComponents = colorAccessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
                bufferColors = reinterpret_cast<const float *>(
                    &(gltfModel.buffers[colorView.buffer].data[colorAccessor.byteOffset + colorView.byteOffset]));
            }
            model.mesh.vertexSize = posAccessor.count;
            for (size_t v = 0; v < posAccessor.count; v++) {
                Vertex vert{};
                vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * 3]), 1.0f);
                vert.normal =
                    glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
                vert.texCoord = bufferTexCoords ? glm::make_vec2(&bufferTexCoords[v * 2]) : glm::vec3(0.0f);
                if (bufferColors) {
                    switch (numColorComponents) {
                        case 3: vert.color = glm::vec4(glm::make_vec3(&bufferColors[v * 3]), 1.0f);
                        case 4: vert.color = glm::make_vec4(&bufferColors[v * 4]);
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

            model.mesh.indicesSize = static_cast<uint32_t>(accessor.count);

            switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    uint32_t *buf = new uint32_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                           accessor.count * sizeof(uint32_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + model.mesh.vertexOffset);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    uint16_t *buf = new uint16_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                           accessor.count * sizeof(uint16_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + model.mesh.vertexOffset);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    uint8_t *buf = new uint8_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                           accessor.count * sizeof(uint8_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + model.mesh.vertexOffset);
                    }
                    break;
                }
                default: throw AssetStorage::AssetStorageException("Index component type not supported!"); break;
            }
        }
        /// End of Indices
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

static AssetStorage::CPUTexture loadGltfTexture(const tinygltf::Image &gltfimage,
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
    return texture;
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
        for (auto &image: gltfModel.images) {
            cpuStorage.textureStaging.add(image.name, loadGltfTexture(image, path.parent_path()));
        }
        for (auto &material: gltfModel.materials) {
            cpuStorage.materialStaging.add(loadGltfMaterial(gltfModel, material));
        }
        for (auto &node: gltfModel.nodes) {
            for (auto &i:
                 loadGltfNode(gltfModel, node, cpuStorage.vertexStagingBuffer, cpuStorage.indexStagingBuffer)) {
                modelStorage.insert(i);
                prefab.modelIds.push_back(i.first);
                meshBoundingBoxStorage.add(i.first, MeshBoundingBox(std::span(cpuStorage.vertexStagingBuffer.begin() +
                                                                                  i.second.mesh.vertexOffset,
                                                                              i.second.mesh.vertexSize)));
            }
        }
    } catch (const AssetStorageException &ase) {
        logger.err("THROW/Asset Storage/GLTF") << "Error while loaded GLTF file :" << ase.what();
        return false;
    }
    prefabStorage.insert(std::make_pair(path.stem().string(), prefab));
    return true;
}

}    // namespace pivot::graphics
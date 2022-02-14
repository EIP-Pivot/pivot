#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ktx.h>
#include <ktxvulkan.h>
#include <span>
#include <tiny_gltf.h>
#include <type_traits>

template <typename T>
constexpr std::int16_t gltf_component_type_code = -1;

template <>
constexpr std::int16_t gltf_component_type_code<std::int8_t> = TINYGLTF_COMPONENT_TYPE_BYTE;
template <>
constexpr std::int16_t gltf_component_type_code<std::uint8_t> = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
template <>
constexpr std::int16_t gltf_component_type_code<std::int16_t> = TINYGLTF_COMPONENT_TYPE_SHORT;
template <>
constexpr std::int16_t gltf_component_type_code<std::uint16_t> = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
template <>
constexpr std::int16_t gltf_component_type_code<std::uint32_t> = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
template <>
constexpr std::int16_t gltf_component_type_code<float> = TINYGLTF_COMPONENT_TYPE_FLOAT;

template <typename T>
requires(gltf_component_type_code<T> != -1) static std::span<const T> getGLTFSpan(const tinygltf::Buffer &buffer,
                                                                                  const tinygltf::BufferView &view,
                                                                                  const tinygltf::Accessor &accessor)
{
    auto &data = buffer.data;
    auto start = accessor.byteOffset + view.byteOffset;
    auto &length = accessor.count;

    if (accessor.componentType != gltf_component_type_code<T>) throw std::logic_error("Invalid component type");
    if (data.size() < start + length) throw std::logic_error("The buffer is too small");
    if (reinterpret_cast<std::uintptr_t>(data.data()) % alignof(T) != 0)
        throw std::logic_error("The buffer not sufficiently aligned");

    return std::span(reinterpret_cast<const T *>(data.data() + start), length);
}

static std::pair<std::span<const float>, tinygltf::Accessor>
getGltfBuffer(const tinygltf::Model &model, const tinygltf::Primitive &primitive, const std::string &name)
{
    DEBUG_FUNCTION

    auto iter = primitive.attributes.find(name);
    if (iter != primitive.attributes.end()) {
        const tinygltf::Accessor &accessor = model.accessors.at(iter->second);
        const tinygltf::BufferView &view = model.bufferViews.at(accessor.bufferView);
        assert(view.byteStride == 0);
        return {getGLTFSpan<float>(model.buffers.at(view.buffer), view, accessor), accessor};
    }
    return {};
}

template <typename T>
requires std::is_unsigned_v<T>
static inline void fillIndexBuffer(const tinygltf::Buffer &buffer, const tinygltf::Accessor &accessor,
                                   const tinygltf::BufferView &bufferView, std::vector<uint32_t> &indexBuffer)
{
    auto buf = getGLTFSpan<T>(buffer, bufferView, accessor);
    indexBuffer.insert(indexBuffer.end(), buf.begin(), buf.end());
}

namespace pivot::graphics
{

static std::vector<std::pair<std::string, AssetStorage::Model>>
loadGltfNode(const tinygltf::Model &gltfModel, const tinygltf::Node &node, std::vector<Vertex> &vertexBuffer,
             std::vector<uint32_t> &indexBuffer, glm::mat4 matrix)
{
    DEBUG_FUNCTION
    logger.debug("Asset Storage/Gltf") << "Loading node: " << node.name;

    if (node.mesh <= -1) return {};
    if (node.translation.size() == 3) {
        matrix = glm::translate(matrix, glm::vec3(glm::make_vec3(node.translation.data())));
    }
    if (node.rotation.size() == 4) {
        glm::quat q = glm::make_quat(node.rotation.data());
        matrix *= glm::mat4(q);
    }
    if (node.scale.size() == 3) { matrix = glm::scale(matrix, glm::vec3(glm::make_vec3(node.scale.data()))); }
    if (node.matrix.size() == 16) { matrix = glm::make_mat4x4(node.matrix.data()); }

    const auto &mesh = gltfModel.meshes.at(node.mesh);
    std::vector<std::pair<std::string, AssetStorage::Model>> loaded;
    for (const auto &i: node.children) {
        auto child = loadGltfNode(gltfModel, gltfModel.nodes.at(i), vertexBuffer, indexBuffer, matrix);
        loaded.insert(loaded.end(), child.begin(), child.end());
    }

    for (const tinygltf::Primitive &primitive: mesh.primitives) {
        AssetStorage::Model model{
            .mesh =
                {
                    .vertexOffset = static_cast<uint32_t>(vertexBuffer.size()),
                    .indicesOffset = static_cast<uint32_t>(indexBuffer.size()),
                },
        };
        // Vertices
        {
            const auto &[positionBuffer, positionAccessor] = getGltfBuffer(gltfModel, primitive, "POSITION");
            const auto &[normalsBuffer, normalAccessor] = getGltfBuffer(gltfModel, primitive, "NORMAL");
            const auto &[texCoordsBuffer, texCoordsAccessor] = getGltfBuffer(gltfModel, primitive, "TEXCOORD_0");
            const auto &[colorBuffer, colorAccessor] = getGltfBuffer(gltfModel, primitive, "COLOR_0");
            auto numColorComponents = colorAccessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;

            assert(!positionBuffer.empty());
            model.mesh.vertexSize = positionBuffer.size();
            for (std::size_t v = 0; v < positionBuffer.size(); v++) {
                Vertex vert{};
                vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f) * matrix;
                vert.normal =
                    normalsBuffer.empty() ? glm::vec3(0.0f) : (glm::normalize(glm::make_vec3(&normalsBuffer[v * 3])));
                vert.texCoord = texCoordsBuffer.empty() ? glm::vec3(0.0f) : glm::make_vec2(&texCoordsBuffer[v * 2]);
                if (false) {
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
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    fillIndexBuffer<std::uint32_t>(buffer, accessor, bufferView, indexBuffer);
                    break;
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    fillIndexBuffer<std::uint16_t>(buffer, accessor, bufferView, indexBuffer);
                    break;
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                    fillIndexBuffer<std::uint8_t>(buffer, accessor, bufferView, indexBuffer);
                    break;
                default: throw AssetStorage::AssetStorageException("Index component type not supported!"); break;
            }
        }
        /// End of Indices
        if (primitive.material > -1) {
            model.default_material = gltfModel.materials.at(primitive.material).name;
        } else {
            model.default_material = "white";
        }
        loaded.push_back(std::make_pair(node.name + std::to_string(model.mesh.vertexOffset), model));
    }
    return loaded;
}

static std::pair<std::string, AssetStorage::CPUMaterial>
loadGltfMaterial(const IndexedStorage<AssetStorage::CPUTexture> &texture, const tinygltf::Model &gltfModel,
                 const tinygltf::Material &mat)
{
    DEBUG_FUNCTION
    AssetStorage::CPUMaterial material;
    if (mat.values.find("baseColorTexture") != mat.values.end()) {
        material.baseColorTexture = texture.getName(mat.values.at("baseColorTexture").TextureIndex());
    }
    // Metallic roughness workflow
    if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
        material.metallicRoughnessTexture = texture.getName(mat.values.at("metallicRoughnessTexture").TextureIndex());
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
        material.normalTexture = texture.getName(mat.additionalValues.at("normalTexture").TextureIndex());
    }
    if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
        material.emissiveTexture = texture.getName(mat.additionalValues.at("emissiveTexture").TextureIndex());
    }
    if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
        material.occlusionTexture = texture.getName(mat.additionalValues.at("occlusionTexture").TextureIndex());
    }
    return std::make_pair(mat.name, material);
}

bool AssetStorage::loadGltfModel(const std::filesystem::path &path)
try {
    DEBUG_FUNCTION

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;
    Prefab prefab;

    bool isLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, path.string());
    if (!warning.empty()) logger.warn("Asset Storage/GLTF") << warning;
    if (!error.empty()) logger.err("Asset Storage/GLTF") << error;

    if (!isLoaded) return false;
    for (const auto &image: gltfModel.images) {
        const auto filepath = path.parent_path() / image.uri;
        loadTexture(filepath);
    }
    for (const auto &material: gltfModel.materials) {
        auto mat = loadGltfMaterial(cpuStorage.textureStaging, gltfModel, material);
        cpuStorage.materialStaging.add(std::move(mat));
    }
    for (const auto &node: gltfModel.nodes) {
        for (const auto &i: loadGltfNode(gltfModel, node, cpuStorage.vertexStagingBuffer, cpuStorage.indexStagingBuffer,
                                         glm::mat4(1.0f))) {
            prefab.modelIds.push_back(i.first);
            modelStorage.insert(i);
        }
    }
    prefabStorage.insert(std::make_pair(path.stem().string(), prefab));
    return true;
} catch (const AssetStorageException &ase) {
    logger.err("THROW/Asset Storage/GLTF") << "Error while loaded GLTF file : " << ase.what();
    return false;
} catch (const std::logic_error &le) {
    logger.err("THROW/Asset Storage/Invalid GLTF file") << "The GLTF file is malformed. Reason :" << le.what();
    return false;
}

}    // namespace pivot::graphics

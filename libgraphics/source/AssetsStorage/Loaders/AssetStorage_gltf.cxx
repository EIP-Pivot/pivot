#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
constexpr std::int16_t gltf_type_code = -1;
template <>
constexpr std::int16_t gltf_type_code<float> = TINYGLTF_TYPE_SCALAR;
template <>
constexpr std::int16_t gltf_type_code<glm::vec2> = TINYGLTF_TYPE_VEC2;
template <>
constexpr std::int16_t gltf_type_code<glm::vec3> = TINYGLTF_TYPE_VEC3;
template <>
constexpr std::int16_t gltf_type_code<glm::vec4> = TINYGLTF_TYPE_VEC4;

template <typename T>
static std::span<const T> getSpan(const tinygltf::Buffer &buffer, const unsigned start, const unsigned length)
{
    auto &data = buffer.data;

    if (data.size() < start + length) throw std::logic_error("The buffer is too small");
    if (reinterpret_cast<std::uintptr_t>(data.data()) % alignof(T) != 0)
        throw std::logic_error("The buffer not sufficiently aligned");

    return std::span(reinterpret_cast<const T *>(data.data() + start), length);
}

template <typename T>
requires(gltf_type_code<T> != -1) std::vector<T> sliceGltfBuffer(const tinygltf::Buffer &buffer,
                                                                 const tinygltf::BufferView &view,
                                                                 const tinygltf::Accessor &accessor)
{
    if (accessor.type != gltf_type_code<T>) throw std::logic_error("Invalid type");
    if (accessor.componentType != gltf_component_type_code<float>) throw std::logic_error("Invalid component type");

    auto ncomp = 1;
    switch (accessor.type) {
        case TINYGLTF_TYPE_SCALAR: ncomp = 1; break;
        case TINYGLTF_TYPE_VEC2: ncomp = 2; break;
        case TINYGLTF_TYPE_VEC3: ncomp = 3; break;
        case TINYGLTF_TYPE_VEC4: ncomp = 4; break;
        default: throw std::logic_error("Invalid type");
    }

    auto byteStride = accessor.ByteStride(view);
    auto start = accessor.byteOffset + view.byteOffset;
    std::vector<T> ret;
    for (unsigned i = 0; i < accessor.count; i++) {
        switch (accessor.componentType) {
            case TINYGLTF_COMPONENT_TYPE_FLOAT: {
                T val;
                const auto data = getSpan<float>(buffer, start + byteStride * i, ncomp);
                for (unsigned i = 0; i < data.size(); ++i) { val[i] = data[i]; }
                ret.push_back(std::move(val));
            } break;
        }
    }
    return ret;
}

template <typename T>
static std::pair<std::vector<T>, tinygltf::Accessor>
getPrimitiveAttribute(const tinygltf::Model &model, const tinygltf::Primitive &primitive, const std::string &name)
{
    DEBUG_FUNCTION

    auto iter = primitive.attributes.find(name);
    if (iter != primitive.attributes.end()) {
        const tinygltf::Accessor &accessor = model.accessors.at(iter->second);
        const tinygltf::BufferView &view = model.bufferViews.at(accessor.bufferView);
        return {sliceGltfBuffer<T>(model.buffers.at(view.buffer), view, accessor), accessor};
    }
    return {};
}

template <typename T>
requires std::is_unsigned_v<T>
static inline void fillIndexBuffer(const tinygltf::Buffer &buffer, const tinygltf::Accessor &accessor,
                                   const tinygltf::BufferView &view, std::vector<uint32_t> &indexBuffer)
{
    auto start = accessor.byteOffset + view.byteOffset;
    auto buf = getSpan<T>(buffer, start, accessor.count);
    indexBuffer.insert(indexBuffer.end(), buf.begin(), buf.end());
}

namespace pivot::graphics::loaders
{

static std::vector<std::pair<std::string, AssetStorage::Model>>
loadGltfNode(const tinygltf::Model &gltfModel, const tinygltf::Node &node, std::vector<Vertex> &vertexBuffer,
             std::vector<uint32_t> &indexBuffer, glm::mat4 matrix)
{
    DEBUG_FUNCTION
    logger.debug("Asset Storage/Gltf") << "Loading node: " << node.name;

    if (node.translation.size() == 3) {
        matrix = glm::translate(matrix, glm::vec3(glm::make_vec3(node.translation.data())));
    }
    if (node.rotation.size() == 4) {
        glm::quat q = glm::make_quat(node.rotation.data());
        matrix *= glm::mat4(q);
    }
    if (node.scale.size() == 3) { matrix = glm::scale(matrix, glm::vec3(glm::make_vec3(node.scale.data()))); }
    if (node.matrix.size() == 16) { matrix = glm::make_mat4x4(node.matrix.data()); }

    std::vector<std::pair<std::string, AssetStorage::Model>> loaded;
    for (const auto &i: node.children) {
        auto child = loadGltfNode(gltfModel, gltfModel.nodes.at(i), vertexBuffer, indexBuffer, matrix);
        loaded.insert(loaded.end(), child.begin(), child.end());
    }
    if (node.mesh <= -1) return loaded;

    const auto &mesh = gltfModel.meshes.at(node.mesh);
    for (const tinygltf::Primitive &primitive: mesh.primitives) {
        /// TODO: support other primitive mode
        if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
            throw AssetStorage::AssetStorageError("Primitive mode not supported !");

        AssetStorage::Model model{
            .mesh =
                {
                    .vertexOffset = static_cast<uint32_t>(vertexBuffer.size()),
                    .indicesOffset = static_cast<uint32_t>(indexBuffer.size()),
                },
        };
        // Vertices
        {
            const auto &[positionBuffer, positionAccessor] =
                getPrimitiveAttribute<glm::vec3>(gltfModel, primitive, "POSITION");
            const auto &[normalsBuffer, normalAccessor] =
                getPrimitiveAttribute<glm::vec3>(gltfModel, primitive, "NORMAL");
            const auto &[texCoordsBuffer, texCoordsAccessor] =
                getPrimitiveAttribute<glm::vec2>(gltfModel, primitive, "TEXCOORD_0");
            const auto &[colorBuffer, colorAccessor] =
                getPrimitiveAttribute<glm::vec3>(gltfModel, primitive, "COLOR_0");

            if (positionBuffer.empty()) throw std::logic_error("No verticies found in a mesh node");
            if (!colorBuffer.empty() && colorAccessor.type != TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3)
                throw AssetStorage::AssetStorageError("Unsupported color type");
            if ((!normalsBuffer.empty() && positionBuffer.size() != normalsBuffer.size()) ||
                (!texCoordsBuffer.empty() && positionBuffer.size() != texCoordsBuffer.size()) ||
                (!colorBuffer.empty() && colorBuffer.size() != texCoordsBuffer.size())) {
                throw std::logic_error("One buffer does not have the apropriate size");
            }

            model.mesh.vertexSize = positionBuffer.size();
            for (unsigned v = 0; v < positionBuffer.size(); v++) {
                Vertex vert;
                vert.pos = glm::vec4(positionBuffer.at(v), 1.0f) * matrix;
                vert.normal = normalsBuffer.empty() ? glm::vec4(0.0f)
                                                    : (glm::normalize(glm::vec4(normalsBuffer.at(v), 1.0f) * matrix));
                vert.texCoord = texCoordsBuffer.empty() ? glm::vec3(0.0f) : texCoordsBuffer.at(v);
                vert.color = colorBuffer.empty() ? glm::vec3(1.0f) : colorBuffer.at(v);
                vertexBuffer.push_back(vert);
            }
        }
        /// End of Verticies

        /// Indices
        {
            const tinygltf::Accessor &accessor = gltfModel.accessors.at(primitive.indices);
            const tinygltf::BufferView &bufferView = gltfModel.bufferViews.at(accessor.bufferView);
            const tinygltf::Buffer &buffer = gltfModel.buffers.at(bufferView.buffer);

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
                default: throw AssetStorage::AssetStorageError("Index component type not supported!"); break;
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
loadGltfMaterial(const IndexedStorage<std::string, AssetStorage::CPUTexture> &texture, const tinygltf::Model &gltfModel,
                 const tinygltf::Material &mat, const unsigned offset)
{
    DEBUG_FUNCTION
    AssetStorage::CPUMaterial material;
    if (mat.values.find("baseColorTexture") != mat.values.end()) {
        material.baseColorTexture = texture.getName(mat.values.at("baseColorTexture").TextureIndex() + offset);
    }
    // Metallic roughness workflow
    if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
        material.metallicRoughnessTexture =
            texture.getName(mat.values.at("metallicRoughnessTexture").TextureIndex() + offset);
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
        material.normalTexture = texture.getName(mat.additionalValues.at("normalTexture").TextureIndex() + offset);
    }
    if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
        material.emissiveTexture = texture.getName(mat.additionalValues.at("emissiveTexture").TextureIndex() + offset);
    }
    if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
        material.occlusionTexture =
            texture.getName(mat.additionalValues.at("occlusionTexture").TextureIndex() + offset);
    }
    return std::make_pair(mat.name, material);
}

bool loadGltfModel(const std::filesystem::path &path, AssetStorage::CPUStorage &storage)
try {
    DEBUG_FUNCTION

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;
    AssetStorage::Prefab prefab;

    bool isLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, path.string());
    if (!warning.empty()) logger.warn("Asset Storage/GLTF") << warning;
    if (!error.empty()) logger.err("Asset Storage/GLTF") << error;
    if (!isLoaded) return false;

    const auto offset = storage.textureStaging.size();
    for (const auto &image: gltfModel.images) {
        const auto filepath = path.parent_path() / image.uri;
        supportedTexture.at(filepath.extension())(filepath, storage);
    }
    for (const auto &material: gltfModel.materials) {
        const auto mat = loadGltfMaterial(storage.textureStaging, gltfModel, material, offset);
        storage.materialStaging.add(std::move(mat));
    }
    if (gltfModel.scenes.empty()) {
        logger.warn("Asset Storage/GLTF") << "GLTF file does not contains scene.";
        return true;
    }
    const auto &scene = gltfModel.scenes[0];
    for (const auto &idx: scene.nodes) {
        const auto &node = gltfModel.nodes.at(idx);
        for (const auto &i:
             loadGltfNode(gltfModel, node, storage.vertexStagingBuffer, storage.indexStagingBuffer, glm::mat4(1.0f))) {
            prefab.modelIds.push_back(i.first);
            storage.modelStorage.insert(i);
        }
    }

    storage.prefabStorage[path.stem().string()] = prefab;
    return true;
} catch (const PivotException &ase) {
    logger.err(ase.getScope()) << "Error while loaded GLTF file : " << ase.what();
    return false;
} catch (const std::logic_error &le) {
    logger.err("THROW/Asset Storage/Invalid GLTF file") << "The GLTF file is malformed. Reason : " << le.what();
    return false;
}

}    // namespace pivot::graphics::loaders

#include "pivot/graphics/AssetStorage/AssetStorage.hxx"

#include "pivot/pivot.hxx"

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
                for (unsigned j = 0; j < data.size(); ++j) { val[j] = data[j]; }
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

static asset::ModelPtr loadGltfNode(const tinygltf::Model &gltfModel, const tinygltf::Node &node,
                                    std::vector<Vertex> &vertexBuffer, std::vector<std::uint32_t> &indexBuffer,
                                    const glm::dmat4 &_matrix)
{
    DEBUG_FUNCTION
    logger.debug("Asset Storage/Gltf") << "Loading node: " << node.name;

    // Can't pass as copy, trigger note about GCC ABI
    glm::dmat4 matrix = _matrix;
    if (node.matrix.size() == 16) {
        matrix *= glm::make_mat4x4(node.matrix.data());
    } else {
        glm::dvec3 translation(0.0f);
        if (node.translation.size() == 3) { translation = glm::make_vec3(node.translation.data()); }

        glm::dquat rotation{};
        if (node.rotation.size() == 4) {
            glm::dquat q = glm::make_quat(node.rotation.data());
            rotation = glm::dmat4(q);
        }

        glm::dvec3 scale(1.0f);
        if (node.scale.size() == 3) { scale = glm::make_vec3(node.scale.data()); }

        matrix *=
            glm::translate(glm::dmat4(1.0f), translation) * glm::dmat4(rotation) * glm::scale(glm::dmat4(1.0f), scale);
    }

    auto loadedNode = std::make_shared<asset::ModelNode>();
    loadedNode->value.name = node.name;
    loadedNode->value.localMatrix = matrix;
    for (const auto &i: node.children) {
        loadedNode->addChild(loadGltfNode(gltfModel, gltfModel.nodes.at(i), vertexBuffer, indexBuffer, matrix));
    }
    if (node.mesh <= -1) return loadedNode;

    const auto &mesh = gltfModel.meshes.at(node.mesh);
    for (const tinygltf::Primitive &primitive: mesh.primitives) {
        /// TODO: support other primitive mode
        if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
            throw AssetStorage::AssetStorageError("Primitive mode not supported !");
        asset::Primitive model{
            .vertexOffset = static_cast<std::uint32_t>(vertexBuffer.size()),
            .vertexSize = 0,
            .indicesOffset = static_cast<std::uint32_t>(indexBuffer.size()),
            .indicesSize = 0,
            .default_material = ((primitive.material > -1) ? (gltfModel.materials.at(primitive.material).name)
                                                           : (asset::missing_material_name)),
            .name = node.name + std::to_string(vertexBuffer.size()),
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
            const auto &[tangentBuffer, tangentAccessor] =
                getPrimitiveAttribute<glm::vec4>(gltfModel, primitive, "TANGENT");

            if (positionBuffer.empty()) throw std::logic_error("No verticies found in a mesh node");
            if (!colorBuffer.empty() && colorAccessor.type != TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3)
                throw AssetStorage::AssetStorageError("Unsupported color type");
            if ((!normalsBuffer.empty() && positionBuffer.size() != normalsBuffer.size()) ||
                (!texCoordsBuffer.empty() && positionBuffer.size() != texCoordsBuffer.size()) ||
                (!colorBuffer.empty() && colorBuffer.size() != texCoordsBuffer.size())) {
                throw std::logic_error("One buffer does not have the apropriate size");
            }

            model.vertexSize = positionBuffer.size();
            for (unsigned v = 0; v < positionBuffer.size(); v++) {
                Vertex vert{
                    .pos = positionBuffer.at(v),
                    .normal = normalsBuffer.empty() ? glm::vec3(0.0f) : (glm::normalize(normalsBuffer.at(v))),
                    .texCoord = texCoordsBuffer.empty() ? glm::vec2(0.0f) : texCoordsBuffer.at(v),
                    .color = colorBuffer.empty() ? glm::vec3(1.0f) : colorBuffer.at(v),
                    .tangent = tangentBuffer.empty() ? glm::vec4(0.0f) : tangentBuffer.at(v),
                };
                vertexBuffer.push_back(vert);
            }
        }
        /// End of Verticies

        /// Indices
        {
            const tinygltf::Accessor &accessor = gltfModel.accessors.at(primitive.indices);
            const tinygltf::BufferView &bufferView = gltfModel.bufferViews.at(accessor.bufferView);
            const tinygltf::Buffer &buffer = gltfModel.buffers.at(bufferView.buffer);

            model.indicesSize += static_cast<std::uint32_t>(accessor.count);

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
        loadedNode->value.primitives.push_back(model);
    }
    return loadedNode;
}

static std::pair<std::string, asset::CPUMaterial> loadGltfMaterial(const std::vector<std::string> &texture,
                                                                   const tinygltf::Material &mat)
{
#define GET_MATERIAL_TEXTURE(value, name)                                                           \
    if (mat.value.find(#name) != mat.value.end()) {                                                 \
        std::size_t idx = mat.value.at(#name).TextureIndex();                                       \
        material.name = (texture.size() > idx) ? (texture.at(idx)) : (asset::missing_texture_name); \
    }
#define GET_MATERIAL_COLOR(value, name)                                           \
    if (mat.value.find(#name) != mat.value.end()) {                               \
        material.name = glm::make_vec4(mat.value.at(#name).ColorFactor().data()); \
    }

#define GET_MATERIAL_FACTOR(value, name) \
    if (mat.value.find(#name) != mat.value.end()) { material.name = static_cast<float>(mat.value.at(#name).Factor()); }

    DEBUG_FUNCTION
    asset::CPUMaterial material;

    GET_MATERIAL_FACTOR(additionalValues, alphaCutOff);

    GET_MATERIAL_FACTOR(values, roughnessFactor);
    GET_MATERIAL_FACTOR(values, metallicFactor);

    GET_MATERIAL_COLOR(values, baseColorFactor);
    GET_MATERIAL_COLOR(additionalValues, emissiveFactor);

    GET_MATERIAL_TEXTURE(values, baseColorTexture);
    GET_MATERIAL_TEXTURE(values, metallicRoughnessTexture);
    GET_MATERIAL_TEXTURE(additionalValues, normalTexture);
    GET_MATERIAL_TEXTURE(additionalValues, occlusionTexture);
    GET_MATERIAL_TEXTURE(additionalValues, emissiveTexture);

#undef GET_MATERIAL_TEXTURE
#undef GET_MATERIAL_COLOR
#undef GET_MATERIAL_FACTOR
    return std::make_pair(mat.name, material);
}

std::optional<asset::CPUStorage> loadGltfModel(const std::filesystem::path &path)
try {
    DEBUG_FUNCTION

    asset::CPUStorage storage;
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;

    bool isLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, path.string());
    if (!warning.empty()) logger.warn("Asset Storage/GLTF") << warning;
    if (!error.empty()) logger.err("Asset Storage/GLTF") << error;
    if (!isLoaded) return std::nullopt;

    std::vector<std::string> texturePath;
    for (const auto &image: gltfModel.images) {
        const auto filepath = path.parent_path() / image.uri;
        storage.texturePaths.emplace(filepath.stem().string(), filepath);
        texturePath.push_back(filepath.stem().string());
    }
    for (const auto &material: gltfModel.materials) {
        const auto mat = loadGltfMaterial(texturePath, material);
        storage.materialStaging.insert(std::move(mat));
    }
    if (gltfModel.scenes.empty()) {
        logger.warn("Asset Storage/GLTF") << "GLTF file does not contains scene.";
        return storage;
    }
    auto prefabNode = std::make_shared<asset::ModelNode>();
    prefabNode->value.name = path.stem().string();
    for (const auto &scene: gltfModel.scenes) {
        auto sceneNode = std::make_shared<asset::ModelNode>();
        sceneNode->value.name = scene.name;
        for (const auto &idx: scene.nodes) {
            const auto &node = gltfModel.nodes.at(idx);
            auto loadedNode =
                loadGltfNode(gltfModel, node, storage.vertexStagingBuffer, storage.indexStagingBuffer, glm::mat4(1.0f));
            sceneNode->addChild(loadedNode);
            storage.modelStorage[loadedNode->value.name] = loadedNode;
        }
        prefabNode->addChild(sceneNode);
        storage.modelStorage[sceneNode->value.name] = sceneNode;
    }
    storage.modelStorage[prefabNode->value.name] = prefabNode;
    return storage;
} catch (const PivotException &ase) {
    logger.err(ase.getScope()) << "Error while loaded GLTF file : " << ase.what();
    return std::nullopt;
} catch (const std::logic_error &le) {
    logger.err("THROW/Asset Storage/Invalid GLTF file") << "The GLTF file is malformed. Reason : " << le.what();
    return std::nullopt;
}

}    // namespace pivot::graphics::loaders

#pragma once

#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/abstract/AImmediateCommand.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"
#include "pivot/graphics/types/IndexedStorage.hxx"
#include "pivot/graphics/types/Material.hxx"
#include "pivot/graphics/types/MeshBoundingBox.hxx"
#include "pivot/graphics/types/Vertex.hxx"
#include "pivot/graphics/types/common.hxx"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace pivot::graphics
{

template <typename T>
/// @brief  Is the type is convertible to filesystem path ?
concept is_valid_path = requires
{
    std::is_convertible_v<T, std::filesystem::path>;
};

/// @brief Store all of the assets used by the game
class AssetStorage
{
public:
    /// @struct AssetStorageException
    /// @brief Exception type for the AssetStorage
    struct AssetStorageException : public std::out_of_range {
        using std::out_of_range::out_of_range;
    };

    /// @brief The function signature of an asset handler
    using AssetHandler = bool (AssetStorage::*)(const std::filesystem::path &);

public:
    /// List of supported texture extensions
    static const std::unordered_map<std::string, AssetHandler> supportedTexture;

    /// List of supported object extensions
    static const std::unordered_map<std::string, AssetHandler> supportedObject;

    /// @brief Represent a mesh in the buffers
    struct Mesh {
        /// Starting offset of the mesh in the vertex buffer
        std::uint32_t vertexOffset;
        /// Number of vertex forming the mesh.
        std::uint32_t vertexSize;
        /// Starting offset of the mesh in the indice buffer
        std::uint32_t indicesOffset;
        /// Number of indice forming the mesh.
        std::uint32_t indicesSize;
    };

    /// @brief Represent a CPU-side material
    struct CPUMaterial {
        /// @cond
        float metallic = 1.0f;
        float roughness = 1.0f;
        glm::vec4 baseColor = glm::vec4(1.0f);
        std::string baseColorTexture;
        std::string metallicRoughnessTexture;
        std::string normalTexture;
        std::string occlusionTexture;
        std::string emissiveTexture;
        ///@endcond
    };

    /// @brief A mesh with a default texture and a default material
    struct Model {
        /// Model mesh
        Mesh mesh;
        /// Default material id
        std::optional<std::string> default_material;
    };

    /// @brief A group of model
    struct Prefab {
        /// The ids of the composing models
        std::vector<std::string> modelIds;
    };

    /// @brief Represent a CPU-side Texture
    struct CPUTexture {
        /// The vulkan image containing the texture
        std::vector<std::byte> image;
        /// The size of the texture
        vk::Extent3D size;
    };

    /// @brief Represent a vulkan texture
    struct Texture {
        /// The vulkan image containing the texture
        AllocatedImage image;
        /// The size of the texture
        vk::Extent3D size;
    };

public:
    /// Constructor
    AssetStorage(VulkanBase &device);
    /// Destructor
    ~AssetStorage();

    template <is_valid_path... Path>
    /// @brief load the 3D models into CPU memory
    ///
    /// @arg the path for all individual file to load
    void loadModels(Path... p)
    {
        auto i = ((loadModel(p)) + ...);
        if (i < sizeof...(Path)) {
            throw AssetStorageException("A model file failed to load. See above for further errors");
        }
    }

    template <is_valid_path... Path>
    /// @brief load the textures into CPU memory
    ///
    /// @arg the path for all individual file to load
    void loadTextures(Path... p)
    {
        auto i = ((loadTexture(p)) + ...);
        if (i < sizeof...(Path)) {
            throw AssetStorageException("A texture file failed to load. See above for further errors");
        }
    }

    /// Push the ressource into GPU memory
    void build();

    /// Free GPU memory
    void destroy();

    template <typename T>
    /// Get an asset of type T named name
    inline const T &get(const std::string &name) const;

    template <typename T>
    /// Get the index of the asset of type T named name
    inline std::int32_t getIndex(const std::string &name) const;

    /// @return Get the Index buffer
    constexpr const AllocatedBuffer &getIndexBuffer() const noexcept { return indicesBuffer; }
    /// @return Get the VertexBuffer
    constexpr const AllocatedBuffer &getVertexBuffer() const noexcept { return vertexBuffer; }

    /// @return Get the Material buffer
    constexpr const AllocatedBuffer &getMaterialBuffer() const noexcept { return materialBuffer; }

    /// @return Get the bounding box buffer
    constexpr const AllocatedBuffer &getBoundingBoxBuffer() const noexcept { return boundingboxbuffer; }

    /// @return Get all the loaded Textures
    constexpr const auto &getTextures() const noexcept { return textureStorage; }

private:
    bool loadModel(const std::filesystem::path &path);
    bool loadObjModel(const std::filesystem::path &path);
    bool loadGltfModel(const std::filesystem::path &path);

    bool loadTexture(const std::filesystem::path &path);
    bool loadPngTexture(const std::filesystem::path &path);
    bool loadKtxImage(const std::filesystem::path &path);

    void pushModelsOnGPU();
    void pushBoundingBoxesOnGPU();
    void pushTexturesOnGPU();
    void pushMaterialOnGPU();

private:
    OptionalRef<VulkanBase> base_ref;

    std::unordered_map<std::string, Model> modelStorage;
    std::unordered_map<std::string, Prefab> prefabStorage;

    IndexedStorage<gpu_object::MeshBoundingBox> meshBoundingBoxStorage;
    IndexedStorage<Texture> textureStorage;
    IndexedStorage<gpu_object::Material> materialStorage;

    struct CPUStorage {
        std::vector<Vertex> vertexStagingBuffer;
        std::vector<std::uint32_t> indexStagingBuffer;
        IndexedStorage<CPUTexture> textureStaging;
        IndexedStorage<CPUMaterial> materialStaging;
    };
    CPUStorage cpuStorage = {};

    AllocatedBuffer vertexBuffer;
    AllocatedBuffer indicesBuffer;
    AllocatedBuffer boundingboxbuffer;
    AllocatedBuffer materialBuffer;
};

#ifndef PIVOT_ASSETSTORAGE_TEMPLATE_INITIALIZED
#define PIVOT_ASSETSTORAGE_TEMPLATE_INITIALIZED

#define PIVOT_TEST_CONTAINS(stor, key) \
    if (!stor.contains(key)) throw AssetStorage::AssetStorageException("Missing " + key + " in " #stor);

template <>
/// @copydoc AssetStorage::get
inline const AssetStorage::Prefab &AssetStorage::get(const std::string &p) const
{
    PIVOT_TEST_CONTAINS(prefabStorage, p);
    return prefabStorage.at(p);
}

template <>
/// @copydoc AssetStorage::get
inline const AssetStorage::Model &AssetStorage::get(const std::string &p) const
{
    PIVOT_TEST_CONTAINS(modelStorage, p);
    return modelStorage.at(p);
}

template <>
/// @copydoc AssetStorage::get
inline const AssetStorage::Mesh &AssetStorage::get(const std::string &p) const
{
    return get<Model>(p).mesh;
}

#undef PIVOT_TEST_CONTAINS

// Get Index of asset in the buffers
template <>
/// @copydoc AssetStorage::get
inline std::int32_t AssetStorage::getIndex<gpu_object::MeshBoundingBox>(const std::string &i) const
{
    return meshBoundingBoxStorage.getIndex(i);
}

template <>
/// @copydoc AssetStorage::get
inline std::int32_t AssetStorage::getIndex<AssetStorage::Texture>(const std::string &i) const
{
    return textureStorage.getIndex(i);
}

template <>
/// @copydoc AssetStorage::get
inline std::int32_t AssetStorage::getIndex<gpu_object::Material>(const std::string &i) const
{
    return materialStorage.getIndex(i);
}

#endif

}    // namespace pivot::graphics

#pragma once

#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/abstract/AImmediateCommand.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"
#include "pivot/graphics/types/Material.hxx"
#include "pivot/graphics/types/MeshBoundingBox.hxx"
#include "pivot/graphics/types/Vertex.hxx"
#include "pivot/graphics/types/common.hxx"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace pivot::graphics
{

template <typename T>
concept is_valid_path = requires
{
    std::is_convertible_v<T, std::filesystem::path>;
};

/// @class AssetStorage
/// Store all of the assets used by the game
class AssetStorage
{
public:
    template <typename T, typename Idx = typename std::vector<T>::size_type>
    /// Store a type in a vector, while keeping a map to the indexes
    class IndexedStorage
    {
    public:
        IndexedStorage() = default;
        ~IndexedStorage() = default;

        /// return an iterator over the indexes
        auto begin() { return index.begin(); }
        /// return the end iterator
        auto end() { return index.end(); }
        /// Add a new item to the storage
        inline void add(const std::string &i, T value)
        {
            storage.push_back(std::move(value));
            index.insert(std::make_pair(i, storage.size() - 1));
        }
        /// @copydoc add
        inline void add(const std::pair<std::string, T> &value) { add(value.first, std::move(value.second)); }
        /// return the number of item in the storage
        constexpr auto size() const noexcept
        {
            assert(storage.size() == index.size());
            return storage.size();
        }
        /// return the internal vector
        constexpr const auto &getStorage() const noexcept { return storage; }
        /// @copydoc getStorage
        constexpr auto &getStorage() noexcept { return storage; }
        /// return the item at a given index
        constexpr const T &get(const std::int32_t &i) const { return storage.at(i); }
        /// return the index of an item name
        inline const std::int32_t getIndex(const std::string &i) const noexcept
        {
            if (index.contains(i))
                return index.at(i);
            else
                return -1;
        }

    private:
        std::vector<T> storage;
        std::unordered_map<std::string, Idx> index;
    };

    /// @struct AssetStorageException
    /// Exception type for the AssetStorage
    struct AssetStorageException : public std::out_of_range {
        using std::out_of_range::out_of_range;
    };

public:
    /// List of supported texture extensions
    static const std::unordered_map<std::string, bool (AssetStorage::*)(const std::filesystem::path &)>
        supportedTexture;

    /// List of supported object extensions
    static const std::unordered_map<std::string, bool (AssetStorage::*)(const std::filesystem::path &)> supportedObject;

    /// @struct Mesh
    /// Represent a mesh in the buffers
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

    /// @struct CPUMaterial
    /// Represent a CPU-side material
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

    /// @struct Material
    /// Represent a GPU side material
    struct Material {
        /// @cond
        float metallic = 1.0f;
        float roughness = 1.0f;
        glm::vec4 baseColor = glm::vec4(1.0f);
        std::int32_t baseColorTexture = -1;
        std::int32_t metallicRoughnessTexture = -1;
        std::int32_t normalTexture = -1;
        std::int32_t occlusionTexture = -1;
        std::int32_t emissiveTexture = -1;
        /// @endcond
    };

    /// A mesh with a default texture and a default material
    struct Model {
        /// Model mesh
        Mesh mesh;
        /// Default material id
        std::optional<std::string> default_material;
    };

    /// An group of model
    struct Prefab {
        /// The ids of the composing models
        std::vector<std::string> modelIds;
    };

    /// @struct CPUTexture
    /// Represent a cpu-side Texture
    struct CPUTexture {
        /// The vulkan image containing the texture
        std::vector<std::byte> image;
        /// The size of the texture
        vk::Extent3D size;
    };

    /// @struct Texture
    /// Represent a vulkan texture
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
    bool loadKtxTexture(const std::filesystem::path &path);

    void pushModelsOnGPU();
    void pushBoundingBoxesOnGPU();
    void pushTexturesOnGPU();
    void pushMaterialOnGPU();

private:
    OptionalRef<VulkanBase> base_ref;

    std::unordered_map<std::string, Model> modelStorage;
    std::unordered_map<std::string, Prefab> prefabStorage;

    IndexedStorage<MeshBoundingBox> meshBoundingBoxStorage;
    IndexedStorage<Texture> textureStorage;
    IndexedStorage<Material> materialStorage;

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
/// @cond
inline const AssetStorage::Prefab &AssetStorage::get(const std::string &p) const
{
    PIVOT_TEST_CONTAINS(prefabStorage, p);
    return prefabStorage.at(p);
}

template <>
inline const AssetStorage::Model &AssetStorage::get(const std::string &p) const
{
    PIVOT_TEST_CONTAINS(modelStorage, p);
    return modelStorage.at(p);
}

template <>
inline const AssetStorage::Mesh &AssetStorage::get(const std::string &p) const
{
    return get<Model>(p).mesh;
}

#undef PIVOT_TEST_CONTAINS

// Get Index of asset in the buffers

template <>
/// @cond
inline std::int32_t AssetStorage::getIndex<MeshBoundingBox>(const std::string &i) const
{
    return meshBoundingBoxStorage.getIndex(i);
}

template <>
inline std::int32_t AssetStorage::getIndex<AssetStorage::Texture>(const std::string &i) const
{
    return textureStorage.getIndex(i);
}

template <>
///@endcond
inline std::int32_t AssetStorage::getIndex<AssetStorage::Material>(const std::string &i) const
{
    return materialStorage.getIndex(i);
}

#endif

}    // namespace pivot::graphics

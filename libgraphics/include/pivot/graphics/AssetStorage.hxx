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
    /// @struct AssetStorageException
    /// Exception type for the AssetStorage
    struct AssetStorageException : public std::out_of_range {
        using std::out_of_range::out_of_range;
    };

public:
    /// List of supported texture extensions
    static constexpr const std::array<const std::string_view, 1> supportedTexture{".png"};
    /// List of supported object extensions
    static constexpr const std::array<const std::string_view, 1> supportedObject{".obj"};

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

    /// A mesh with a default texture and a default material
    struct Model {
        /// Model mesh
        Mesh mesh;
        /// Default texture id
        std::optional<std::string> default_texture;
        /// Default material id
        std::optional<std::string> default_material;
    };

    /// An group of model
    struct Prefab {
        /// The ids of the composing models
        std::vector<std::string> modelIds;
    };

    /// @struct Texture
    /// Represent a Texture
    struct Texture {
        /// The vulkan image containing the texture
        std::variant<AllocatedImage, std::vector<std::byte>> image;
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
    /// @return the number of file successfully loaded
    unsigned loadModels(Path... p)
    {
        return ((loadModel(p)) + ...);
    }

    template <is_valid_path... Path>
    /// @brief load the textures into CPU memory
    ///
    /// @arg the path for all individual file to load
    /// @return the number of file successfully loaded
    unsigned loadTextures(Path... p)
    {
        return ((loadTexture(p)) + ...);
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
    inline std::uint32_t getIndex(const std::string &name) const;

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
    bool loadTexture(const std::filesystem::path &path);
    bool loadMaterial(const tinyobj::material_t &material);
    void pushModelsOnGPU();
    void pushBoundingBoxesOnGPU();
    void pushTexturesOnGPU();
    void pushMaterialOnGPU();

private:
    OptionalRef<VulkanBase> base_ref;
    std::unordered_map<std::string, Model> modelStorage;
    std::unordered_map<std::string, Prefab> prefabStorage;
    std::unordered_map<std::string, MeshBoundingBox> meshBoundingBoxStorage;
    std::unordered_map<std::string, Texture> textureStorage;
    std::unordered_map<std::string, gpuObject::Material> materialStorage;

    std::vector<Vertex> vertexStagingBuffer;
    std::vector<std::uint32_t> indexStagingBuffer;

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

template <>
inline const MeshBoundingBox &AssetStorage::get(const std::string &p) const
{
    PIVOT_TEST_CONTAINS(meshBoundingBoxStorage, p);
    return meshBoundingBoxStorage.at(p);
}

template <>
inline const AssetStorage::Texture &AssetStorage::get(const std::string &p) const
{
    PIVOT_TEST_CONTAINS(textureStorage, p);
    return textureStorage.at(p);
}

// Get Index of asset in the buffers

template <>
/// @cond
inline std::uint32_t AssetStorage::getIndex<AssetStorage::Model>(const std::string &i) const
{
    PIVOT_TEST_CONTAINS(modelStorage, i);
    return std::distance(modelStorage.begin(), modelStorage.find(i));
}

template <>
inline std::uint32_t AssetStorage::getIndex<AssetStorage::Mesh>(const std::string &i) const
{
    return getIndex<AssetStorage::Model>(i);
}

template <>
inline std::uint32_t AssetStorage::getIndex<MeshBoundingBox>(const std::string &i) const
{
    PIVOT_TEST_CONTAINS(meshBoundingBoxStorage, i);
    return std::distance(meshBoundingBoxStorage.begin(), meshBoundingBoxStorage.find(i));
}

template <>
inline std::uint32_t AssetStorage::getIndex<AssetStorage::Texture>(const std::string &i) const
{
    PIVOT_TEST_CONTAINS(textureStorage, i);
    return std::distance(textureStorage.begin(), textureStorage.find(i));
}

template <>
inline std::uint32_t AssetStorage::getIndex<gpuObject::Material>(const std::string &i) const
{
    PIVOT_TEST_CONTAINS(materialStorage, i);
    return std::distance(materialStorage.begin(), materialStorage.find(i));
}

///@endcond

#undef PIVOT_TEST_CONTAINS

#endif

}    // namespace pivot::graphics

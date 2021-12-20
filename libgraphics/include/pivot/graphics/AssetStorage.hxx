#pragma once

#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/abstract/AImmediateCommand.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/Material.hxx"
#include "pivot/graphics/types/Mesh.hxx"
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
concept is_valid_path = requires
{
    std::is_convertible_v<T, std::filesystem::path>;
};

class AssetStorage
{
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

        /// Default texture id
        std::optional<std::string> default_texture;
        /// Default material id
        std::optional<std::string> default_material;
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
    /// @arg paths the path for all individual file to load
    /// @return the number of file successfully loaded
    unsigned loadModels(Path... p)
    {
        return ((loadModel(p)) + ...);
    }

    template <is_valid_path... Path>
    /// @brief load the 3D models into CPU memory
    ///
    /// @arg the path for all individual file to load
    /// @return the number of file successfully loaded
    unsigned loadTextures(Path... p)
    {
        return ((loadTexture(p)) + ...);
    }

    void build();
    void destroy();

    template <typename T>
    inline const T &get(const std::string &) const;

    template <typename T>
    inline std::uint32_t getIndex(const std::string &i) const;

    constexpr const AllocatedBuffer &getIndexBuffer() const noexcept { return indicesBuffer; }
    constexpr const AllocatedBuffer &getVertexBuffer() const noexcept { return vertexBuffer; }

    constexpr const AllocatedBuffer &getMaterialBuffer() const noexcept { return materialBuffer; }
    inline const std::size_t getMaterialBufferSize() const noexcept { return materialStorage.size(); }

    constexpr const auto &getTextures() const noexcept { return textureStorage; }

private:
    bool loadModel(const std::filesystem::path &path);
    bool loadTexture(const std::filesystem::path &path);
    void pushModelsOnGPU();
    void pushTexturesOnGPU();
    void pushMaterialOnGPU();

private:
    OptionalRef<VulkanBase> base_ref;
    std::unordered_map<std::string, Mesh> meshStorage;
    std::unordered_map<std::string, MeshBoundingBox> meshBoundingBoxStorage;
    std::unordered_map<std::string, Texture> textureStorage;
    std::unordered_map<std::string, gpuObject::Material> materialStorage;

    std::vector<Vertex> vertexStagingBuffer;
    std::vector<std::uint32_t> indexStagingBuffer;

    AllocatedBuffer vertexBuffer;
    AllocatedBuffer indicesBuffer;
    AllocatedBuffer materialBuffer;
};

#ifndef PIVOT_ASSETSTORAGE_TEMPLATE_INITIALIZED
#define PIVOT_ASSETSTORAGE_TEMPLATE_INITIALIZED

/// Get the assets for give id
template <>
inline const AssetStorage::Mesh &AssetStorage::get(const std::string &p) const
{
    return meshStorage.at(p);
}

template <>
inline const AssetStorage::Texture &AssetStorage::get(const std::string &p) const
{
    return textureStorage.at(p);
}

template <>
inline const MeshBoundingBox &AssetStorage::get(const std::string &p) const
{
    return meshBoundingBoxStorage.at(p);
}

// Get Index of asset in the buffers
template <>
inline std::uint32_t AssetStorage::getIndex<AssetStorage::Mesh>(const std::string &i) const
{
    return std::distance(meshStorage.begin(), meshStorage.find(i));
}

template <>
inline std::uint32_t AssetStorage::getIndex<AssetStorage::Texture>(const std::string &i) const
{
    return std::distance(textureStorage.begin(), textureStorage.find(i));
}

template <>
inline std::uint32_t AssetStorage::getIndex<gpuObject::Material>(const std::string &i) const
{
    return std::distance(materialStorage.begin(), materialStorage.find(i));
}

#endif

}    // namespace pivot::graphics

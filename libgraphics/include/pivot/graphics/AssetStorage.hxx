#pragma once

#include "pivot/graphics/DeletionQueue.hxx"
#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/abstract/AImmediateCommand.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"
#include "pivot/graphics/types/Character.hxx"
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

    /// List of supported font extensions
    static const std::unordered_map<std::string, AssetHandler> supportedFonts;

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

    struct CPUCharacter {
        std::string textureId;
        glm::ivec2 bearing;
        unsigned advance;
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
        vk::Extent3D size = {0, 0, 1};
        /// The format of the texture
        vk::Format format = vk::Format::eR8G8B8A8Srgb;
    };

    /// Alias for AllocatedImage
    using Texture = AllocatedImage;

public:
    /// Constructor
    AssetStorage(VulkanBase &device);
    /// Destructor
    ~AssetStorage();

    template <class... Path>
    /// @brief load the 3D models into CPU memory
    ///
    /// @arg the path for all individual file to load
    requires std::is_convertible_v<Path..., std::filesystem::path>
    void loadModels(Path... p)
    {
        unsigned i = ((loadModel(p)) + ...);
        if (i < sizeof...(Path))
            throw AssetStorageException("A model file failed to load. See above for further errors");
    }
    /// Load a single model file
    bool loadModel(const std::filesystem::path &path);

    template <class... Path>
    /// @brief load the textures into CPU memory
    ///
    /// @arg the path for all individual file to load
    requires std::is_convertible_v<Path..., std::filesystem::path>
    void loadTextures(Path... p)
    {
        unsigned i = ((loadTexture(p)) + ...);
        if (i < sizeof...(Path))
            throw AssetStorageException("A texture file failed to load. See above for further errors");
    }
    /// Load a single texture
    bool loadTexture(const std::filesystem::path &path);

    template <class... Path>
    /// @brief load the font into CPU memory
    ///
    /// @arg the path for all individual file to load
    requires std::is_convertible_v<Path..., std::filesystem::path>
    void loadFonts(Path... p)
    {
        unsigned i = ((loadFont(p)) + ...);
        if (i < sizeof...(Path))
            throw AssetStorageException("A font file failed to load. See above for further errors");
    }
    /// Load a single font
    bool loadFont(const std::filesystem::path &path);

    /// @brief add a material into cpu storage
    void addMaterial(const std::string &name, CPUMaterial material);

    /// Push the ressource into GPU memory
    void build();

    /// Free GPU memory
    void destroy();

    /// @brief Return the descriptor set layout of the asset storage
    ///
    /// This is the corresponding code in glsl code
    /// @code {.glsl}
    /// struct BoundingBox {
    ///     vec3 low;
    ///     vec3 high;
    /// };
    /// struct Material {
    ///     float metallic;
    ///     float roughness;
    ///     vec4 baseColor;
    ///     int baseColorTexture;
    ///     int metallicRoughnessTexture;
    ///     int normalTexture;
    ///     int occlusionTexture;
    ///     int emissiveTexture;
    /// };
    ///
    /// layout(set = 0, binding = 0) readonly buffer ObjectBoundingBoxes{
    ///     BoundingBox boundingBoxes[];
    /// } objectBoundingBoxes;
    ///
    /// layout (std140, set = 0, binding = 1) readonly buffer ObjectMaterials {
    ///     Material materials[];
    /// } objectMaterials;
    ///
    /// layout (set = 0, binding = 2) sampler2D texSampler[];
    /// @endcode
    const auto &getDescriptorSetLayout() const noexcept { return descriptorSetLayout; }
    /// @brief Bind the vertex buffer, indices buffer and asset descriptor set on the provided command buffer
    ///
    /// Every ressource will be bind on vk::PipelineBindPoint::eGraphics
    ///
    /// @arg The command buffer used to bind the ressources
    /// @arg The layout of the pipeline to use. Must be created using the Asset Storage descriptor set
    /// layout to avoid validation layers erros
    /// @arg The index of the descriptor set to be bind on
    bool bindForGraphics(vk::CommandBuffer &cmd, const vk::PipelineLayout &pipelineLayout,
                         std::uint32_t descriptorSet = 0, bool bindVertex = true, bool bindIndex = true);
    /// @brief Bind the asset descriptor set on the provided command buffer
    ///
    /// Every ressource will be bind on vk::PipelineBindPoint::eCulling
    ///
    /// @arg The command buffer used to bind the ressources
    /// @arg The layout of the pipeline to use. Must be created using the Asset Storage descriptor set
    /// layout to avoid validation layers erros
    /// @arg The index of the descriptor set to be bind on
    bool bindForCompute(vk::CommandBuffer &cmd, const vk::PipelineLayout &pipelineLayout,
                        std::uint32_t descriptorSet = 0);

    template <typename T>
    /// Get an asset of type T named name
    inline const T &get(const std::string &name) const;

    const gpu_object::Character &getChar(const char &p) const { return charStorage.get(p); }

    template <typename T>
    /// Get an asset of type T named name if it exists
    inline OptionalRef<const T> get_optional(const std::string &name) const;

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
    constexpr const AllocatedBuffer &getBoundingBoxBuffer() const noexcept { return boundingboxBuffer; }

    /// @return Get all the loaded Textures
    constexpr const auto &getTextures() const noexcept { return textureStorage; }

private:
    /// Load models
    bool loadObjModel(const std::filesystem::path &path);
    bool loadGltfModel(const std::filesystem::path &path);

    /// Load texture
    bool loadPngTexture(const std::filesystem::path &path);
    bool loadKtxImage(const std::filesystem::path &path);

    // Load font
    bool loadTTFFont(const std::filesystem::path &path);
    // Push to gpu

    void pushModelsOnGPU();
    void pushBoundingBoxesOnGPU();
    void pushTexturesOnGPU();
    void pushMaterialOnGPU();
    void pushCharactersOnGPU();

    // Descriptor ressources
    void createTextureSampler();
    void createDescriptorPool();
    void createDescriptorSetLayout();
    void createDescriptorSet();

private:
    OptionalRef<VulkanBase> base_ref;

    // Abstract ressouces
    std::unordered_map<std::string, Model> modelStorage;
    std::unordered_map<std::string, Prefab> prefabStorage;

    // Buffers
    IndexedStorage<gpu_object::Character> charStorage;
    IndexedStorage<gpu_object::MeshBoundingBox> meshBoundingBoxStorage;
    IndexedStorage<Texture> textureStorage;
    IndexedStorage<gpu_object::Material> materialStorage;

    // CPU-side storage
    struct CPUStorage {
        std::vector<Vertex> vertexStagingBuffer;
        std::vector<std::uint32_t> indexStagingBuffer;
        IndexedStorage<CPUCharacter> characterStaging;
        IndexedStorage<CPUTexture> textureStaging;
        IndexedStorage<CPUMaterial> materialStaging;
    } cpuStorage = {};

    // Vulkan Ressouces
    DeletionQueue vulkanDeletionQueue;
    vk::Sampler textureSampler;
    vk::DescriptorPool descriptorPool;
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSet descriptorSet;
    AllocatedBuffer vertexBuffer;
    AllocatedBuffer indicesBuffer;
    AllocatedBuffer boundingboxBuffer;
    AllocatedBuffer materialBuffer;
    AllocatedBuffer characterBuffer;
};

#ifndef PIVOT_ASSETSTORAGE_TEMPLATE_INITIALIZED
#define PIVOT_ASSETSTORAGE_TEMPLATE_INITIALIZED

#define PIVOT_TEST_CONTAINS(stor, key) \
    if (!stor.contains(key)) throw AssetStorage::AssetStorageException("Missing " + key + " in " #stor);

template <>
/// @copydoc AssetStorage::get
inline const AssetStorage::Texture &AssetStorage::get(const std::string &p) const
{
    PIVOT_TEST_CONTAINS(textureStorage, p);
    return textureStorage.get(p);
}

template <>
/// @copydoc AssetStorage::get
inline const gpu_object::Character &AssetStorage::get(const std::string &p) const
{
    PIVOT_TEST_CONTAINS(charStorage, p);
    return charStorage.get(p);
}

template <>
/// @copydoc AssetStorage::get
inline const AssetStorage::Prefab &AssetStorage::get(const std::string &p) const
{
    PIVOT_TEST_CONTAINS(prefabStorage, p);
    return prefabStorage.at(p);
}

template <>
/// @copydoc AssetStorage::get_optional
inline OptionalRef<const AssetStorage::Prefab> AssetStorage::get_optional(const std::string &p) const
{
    auto prefab = prefabStorage.find(p);
    if (prefab == prefabStorage.end()) return std::nullopt;
    return prefab->second;
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

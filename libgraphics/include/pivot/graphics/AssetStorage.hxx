#pragma once

#include "pivot/containers/IndexedStorage.hxx"
#include "pivot/graphics/DeletionQueue.hxx"
#include "pivot/graphics/DescriptorAllocator/DescriptorBuilder.hxx"
#include "pivot/graphics/ThreadPool.hxx"
#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/VulkanImmediateCommand.hxx"
#include "pivot/graphics/types/AABB.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"
#include "pivot/graphics/types/Material.hxx"
#include "pivot/graphics/types/Vertex.hxx"
#include "pivot/graphics/types/common.hxx"
#include "pivot/utility/flags.hxx"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#ifndef PIVOT_ASSET_DEFAULT_DIRECTORY
    #define PIVOT_ASSET_DEFAULT_DIRECTORY "."
#endif

namespace pivot::graphics
{

template <typename T>
/// @brief Is the type convertible to filesystem path ?
concept is_valid_path = requires
{
    std::is_convertible_v<T, std::filesystem::path>;
};

/// @brief Store all of the assets used by the game
class AssetStorage
{
public:
    /// Error type for the AssetStorage
    LOGIC_ERROR(AssetStorage);

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
        /// Equality operator
        constexpr bool operator==(const Mesh &other) const = default;
    };

    /// @brief Represent a CPU-side material
    struct CPUMaterial {
        /// @cond
        float alphaCutOff = 1.0f;
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        glm::vec4 baseColor = glm::vec4(1.0f);
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        glm::vec4 emissiveFactor = glm::vec4(1.0f);
        std::string baseColorTexture = missing_texture_name;
        std::string metallicRoughnessTexture = "";
        std::string normalTexture = "";
        std::string occlusionTexture = "";
        std::string emissiveTexture = "";
        std::string specularGlossinessTexture = "";
        std::string diffuseTexture = "";
        bool operator==(const CPUMaterial &) const = default;
        ///@endcond
    };

    /// @brief A mesh with a default texture and a default material
    struct Model {
        /// Model mesh
        Mesh mesh;
        /// Default material id
        std::optional<std::string> default_material;
        /// Equality operator
        bool operator==(const Model &) const = default;
    };

    /// @brief A group of model
    struct Prefab {
        /// The ids of the composing models
        std::vector<std::string> modelIds;
        /// Equality operator
        bool operator==(const Prefab &) const = default;
    };

    /// @brief Represent a CPU-side Texture
    struct CPUTexture {
        /// The vulkan image containing the texture
        std::vector<std::byte> image;
        /// The size of the texture
        vk::Extent3D size;
        /// Equality operator
        bool operator==(const CPUTexture &) const = default;
    };

    /// Alias for AllocatedImage
    using Texture = AllocatedImage;

    /// Select how to handle the ressouces
    enum class BuildFlagBits : FlagsType {
        eClear = BIT(0),
        eReloadOldAssets = BIT(1),
    };
    /// @copydoc BuildFlagBits
    using BuildFlags = Flags<BuildFlagBits>;

    /// Represent the loaded assets before being uploaded to the GPU
    class CPUStorage
    {
    public:
        /// CPUStorage error type
        LOGIC_ERROR(CPUStorage);

        /// Merge two CPUStorage together
        void merge(const CPUStorage &other);

        /// Operator += overload
        CPUStorage &operator+=(const CPUStorage &other);
        /// Equality operator
        bool operator==(const CPUStorage &) const = default;

    public:
        /// Return a CPUStorage with default asset
        static CPUStorage default_assets();

    public:
        /// store the Models
        std::unordered_map<std::string, Model> modelStorage;
        /// store the Prefab
        std::unordered_map<std::string, Prefab> prefabStorage;
        /// Store the Vertex
        std::vector<Vertex> vertexStagingBuffer;
        /// Store the index of the buffer
        std::vector<std::uint32_t> indexStagingBuffer;
        /// Store the textures
        IndexedStorage<std::string, CPUTexture> textureStaging;
        /// Store the Materials
        IndexedStorage<std::string, CPUMaterial> materialStaging;
        /// Store the path of the model currently loaded
        std::unordered_map<std::string, std::filesystem::path> modelPaths;
        /// Store the path of the texture currently loaded
        std::unordered_map<std::string, std::filesystem::path> texturePaths;
    };

    /// name of the fallback texture if missing
    static constexpr auto missing_texture_name = "internal/missing_texture";
    /// data of the default texture
    static const CPUTexture default_texture_data;
    /// name of the default material if missing
    static constexpr auto missing_material_name = "internal/missing_material";
    /// name of the default quad mesh
    static constexpr auto quad_mesh = "internal/quad_mesh";
    /// Vertices of the default mesh
    static const std::vector<Vertex> quad_vertices;
    /// Indeices of the default mesh
    static const std::vector<std::uint32_t> quad_indices;

public:
    /// Constructor
    AssetStorage(VulkanBase &device);
    AssetStorage(const AssetStorage &) = delete;
    /// Destructor
    ~AssetStorage();

    /// Set the asset directory
    void setAssetDirectory(const std::filesystem::path &path) noexcept { asset_dir = path; }

    /// Load a single assets file
    bool addAsset(const std::filesystem::path &path);
    /// @copydoc addAsset
    bool addAsset(const std::vector<std::filesystem::path> &path);

    /// Load a single model file
    bool addModel(const std::filesystem::path &path);
    /// @copydoc addModel
    bool addModel(const std::vector<std::filesystem::path> &path);

    /// Load a single texture
    bool addTexture(const std::filesystem::path &path);
    /// @copydoc addTexture
    bool addTexture(const std::vector<std::filesystem::path> &path);

    /// Push the ressource into GPU memory
    void build(DescriptorBuilder builder, BuildFlags flags = BuildFlagBits::eClear);

    /// Free GPU memory
    void destroy();

    /// @brief Return the descriptor set layout of the asset storage
    ///
    /// This is the corresponding code in glsl code
    /// @code{.glsl}
    /// struct AABB {
    ///     vec3 low;
    ///     vec3 high;
    /// };

    /// struct Material {
    ///     float alphaCutOff;
    ///     float metallic;
    ///     float roughness;
    ///     vec4 baseColor;
    ///     vec4 baseColorFactor;
    ///     vec4 emissiveFactor;
    ///     int baseColorTexture;
    ///     int metallicRoughnessTexture;
    ///     int normalTexture;
    ///     int occlusionTexture;
    ///     int emissiveTexture;
    ///     int specularGlossinessTexture;
    ///     int diffuseTexture;
    /// };
    ///
    /// layout(set = 0, binding = 0) readonly buffer ObjectAABB{
    ///     AABB boundingBoxes[];
    /// } objectAABB;
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
                         std::uint32_t descriptorSet = 0);
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

    /// Return the path of all the models currently loaded in the CPU Storage
    auto getModelPaths() const
    {
        return this->modelPaths | std::views::transform([](const auto &i) { return i.second; });
    }
    /// Return the path of the model given in argument
    const std::optional<std::filesystem::path> getModelPath(const std::string &name) const
    {
        auto it = modelPaths.find(name);
        if (it == modelPaths.end()) { return std::nullopt; }
        return {it->second};
    }
    /// Return the path of all the models currently loaded in the CPU Storage
    auto getTexturePaths() const
    {
        return this->texturePaths | std::views::transform([](const auto &i) { return i.second; });
    }
    /// Return the path of the texture given in argument
    const std::optional<std::filesystem::path> getTexturePath(const std::string &name) const
    {
        auto it = texturePaths.find(name);
        if (it == texturePaths.end()) { return std::nullopt; }
        return {it->second};
    }

    /// Return the name of all the texture currently loaded in the Storage
    auto getTextures() const
    {
        return this->textureStorage | std::views::transform([](const auto &i) { return i.first; });
    }

    /// Return the name of all the models currently loaded in the Storage
    auto getModels() const
    {
        return this->modelStorage | std::views::transform([](const auto &i) { return i.first; });
    }

    /// Return the path of all the prefabs currently loaded in the Storage
    auto getPrefabs() const
    {
        return this->prefabStorage | std::views::transform([](const auto &i) { return i.first; });
    }

    /// Return the path of all the models currently loaded in the Storage
    auto getMaterial() const
    {
        return this->materialStorage | std::views::transform([](const auto &i) { return i.first; });
    }

    template <typename T>
    /// Get an asset of type T named name
    inline const T &get(const std::string &name) const;

    template <typename T>
    /// Return the amount of ressources
    inline std::uint32_t getSize() const;

    template <typename T>
    /// Get an asset of type T named name if it exists
    inline OptionalRef<const T> get_optional(const std::string &name) const;

    template <typename T>
    /// Get the index of the asset of type T named name
    inline std::int32_t getIndex(const std::string &name) const;

private:
    /// Load models
    static std::optional<CPUStorage> loadModel(unsigned, const std::filesystem::path &path);

    /// Load texture
    static std::optional<CPUStorage> loadTexture(unsigned, const std::filesystem::path &path);

    // Push to gpu
    void pushModelsOnGPU();
    void pushTexturesOnGPU();
    void pushMaterialOnGPU();

    // Descriptor ressources
    void createTextureSampler();
    void createDescriptorSet(DescriptorBuilder &builder);

private:
    OptionalRef<VulkanBase> base_ref;
    std::filesystem::path asset_dir = PIVOT_ASSET_DEFAULT_DIRECTORY;
    ThreadPool threadPool;

    // Abstract ressouces
    std::unordered_map<std::string, Model> modelStorage;
    std::unordered_map<std::string, Prefab> prefabStorage;

    // Buffers
    IndexedStorage<std::string, gpu_object::AABB> meshAABBStorage;
    IndexedStorage<std::string, Texture> textureStorage;
    IndexedStorage<std::string, gpu_object::Material> materialStorage;

    // CPU-side storage
    CPUStorage cpuStorage = {};
    std::unordered_map<std::string, std::filesystem::path> modelPaths;
    std::unordered_map<std::string, std::filesystem::path> texturePaths;

    // Vulkan Ressouces
    DeletionQueue vulkanDeletionQueue;
    vk::Sampler textureSampler;
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSet descriptorSet;
    AllocatedBuffer<Vertex> vertexBuffer;
    AllocatedBuffer<std::uint32_t> indicesBuffer;
    AllocatedBuffer<gpu_object::AABB> AABBBuffer;
    AllocatedBuffer<gpu_object::Material> materialBuffer;
};

namespace loaders
{
    /// @brief The function signature of an asset handler
    using AssetHandler = std::function<std::optional<AssetStorage::CPUStorage>(const std::filesystem::path &)>;

    /// Load a .obj file
    std::optional<AssetStorage::CPUStorage> loadObjModel(const std::filesystem::path &path);
    /// Load a .gltf file
    std::optional<AssetStorage::CPUStorage> loadGltfModel(const std::filesystem::path &path);

    /// Load a .png file
    std::optional<AssetStorage::CPUStorage> loadPngTexture(const std::filesystem::path &path);
    /// Load a .jpg file
    std::optional<AssetStorage::CPUStorage> loadJpgTexture(const std::filesystem::path &path);
    /// Load a .ktx file
    std::optional<AssetStorage::CPUStorage> loadKtxImage(const std::filesystem::path &path);

    /// List of supported texture extensions
    const std::unordered_map<std::string, loaders::AssetHandler> supportedTexture = {
        {".png", &loadPngTexture},
        {".jpg", &loadJpgTexture},
        {".ktx", &loadKtxImage},
    };

    /// List of supported object extensions
    const std::unordered_map<std::string, AssetHandler> supportedObject = {
        {".obj", &loadObjModel},
        {".gltf", &loadGltfModel},
    };

}    // namespace loaders

#ifndef PIVOT_ASSETSTORAGE_TEMPLATE_INITIALIZED
    #define PIVOT_ASSETSTORAGE_TEMPLATE_INITIALIZED

    #define PIVOT_TEST_CONTAINS(stor, key) \
        if (!stor.contains(key)) throw AssetStorage::AssetStorageError("Missing " + key + " in " #stor);

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
/// @copydoc AssetStorage::get_optional
inline OptionalRef<const gpu_object::AABB> AssetStorage::get_optional(const std::string &p) const
{
    if (meshAABBStorage.contains(p)) { return std::make_optional(std::ref(meshAABBStorage.get(p))); }
    return std::nullopt;
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
inline std::int32_t AssetStorage::getIndex<gpu_object::AABB>(const std::string &i) const
{
    return meshAABBStorage.getIndex(i);
}

template <>
/// @copydoc AssetStorage::get
inline std::int32_t AssetStorage::getIndex<AssetStorage::Texture>(const std::string &i) const
{
    auto idx = textureStorage.getIndex(i);
    if (idx == -1) return getIndex<Texture>(missing_texture_name);
    return idx;
}

template <>
/// @copydoc AssetStorage::get
inline std::int32_t AssetStorage::getIndex<gpu_object::Material>(const std::string &i) const
{
    auto idx = materialStorage.getIndex(i);
    if (idx == -1) return getIndex<gpu_object::Material>(missing_material_name);
    return idx;
}

template <>
/// @copydoc AssetStorage::getSize
inline std::uint32_t AssetStorage::getSize<gpu_object::Material>() const
{
    return materialStorage.size();
}

template <>
/// @copydoc AssetStorage::getSize
inline std::uint32_t AssetStorage::getSize<AssetStorage::Texture>() const
{
    return textureStorage.size();
}

#endif

}    // namespace pivot::graphics

ENABLE_FLAGS_FOR_ENUM(pivot::graphics::AssetStorage::BuildFlagBits);

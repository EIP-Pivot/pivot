#pragma once

#include "pivot/containers/IndexedStorage.hxx"
#include "pivot/containers/Node.hxx"
#include "pivot/graphics/AssetStorage/CPUMaterial.hxx"
#include "pivot/graphics/AssetStorage/CPUStorage.hxx"
#include "pivot/graphics/AssetStorage/CPUTexture.hxx"
#include "pivot/graphics/AssetStorage/DefaultRessources.hxx"
#include "pivot/graphics/AssetStorage/Model.hxx"
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

    /// Alias for AllocatedImage
    using Texture = AllocatedImage;

    /// Select how to handle the ressouces
    enum class BuildFlagBits : FlagsType {
        eClear = BIT(0),
        eReloadOldAssets = BIT(1),
    };
    /// @copydoc BuildFlagBits
    using BuildFlags = Flags<BuildFlagBits>;

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

    /// Return the name of all the models currently loaded in the Storage
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
    static std::optional<asset::CPUStorage> loadModel(unsigned, const std::filesystem::path &path);

    /// Load texture
    static std::optional<asset::CPUStorage> loadTexture(unsigned, const std::filesystem::path &path);

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
    std::unordered_map<std::string, asset::ModelPtr> modelStorage;

    // Buffers
    IndexedStorage<std::string, gpu_object::AABB> meshAABBStorage;
    IndexedStorage<std::string, Texture> textureStorage;
    IndexedStorage<std::string, gpu_object::Material> materialStorage;

    // CPU-side storage
    asset::CPUStorage cpuStorage = asset::CPUStorage::default_assets();
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

#ifndef PIVOT_ASSETSTORAGE_TEMPLATE_INITIALIZED
    #define PIVOT_ASSETSTORAGE_TEMPLATE_INITIALIZED

    #define PIVOT_TEST_CONTAINS(stor, key) \
        if (!stor.contains(key)) throw AssetStorage::AssetStorageError("Missing " + key + " in " #stor);

template <>
/// @copydoc AssetStorage::get
inline const asset::ModelPtr &AssetStorage::get(const std::string &p) const
{
    PIVOT_TEST_CONTAINS(modelStorage, p);
    return modelStorage.at(p);
}
    #undef PIVOT_TEST_CONTAINS

template <>
/// @copydoc AssetStorage::get_optional
inline OptionalRef<const gpu_object::AABB> AssetStorage::get_optional(const std::string &p) const
{
    if (meshAABBStorage.contains(p)) { return std::make_optional(std::ref(meshAABBStorage.at(p))); }
    return std::nullopt;
}

template <>
/// @copydoc AssetStorage::get
inline const asset::Model &AssetStorage::get(const std::string &p) const
{
    const auto &ptr = get<asset::ModelPtr>(p);
    return ptr->value;
}

template <>
/// @copydoc AssetStorage::get_optional
inline OptionalRef<const asset::ModelPtr> AssetStorage::get_optional(const std::string &p) const
{
    auto model = modelStorage.find(p);
    if (model == modelStorage.end()) return std::nullopt;
    return model->second;
}

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
    if (idx == -1) return getIndex<Texture>(asset::missing_texture_name);
    return idx;
}

template <>
/// @copydoc AssetStorage::get
inline std::int32_t AssetStorage::getIndex<gpu_object::Material>(const std::string &i) const
{
    auto idx = materialStorage.getIndex(i);
    if (idx == -1) return getIndex<gpu_object::Material>(asset::missing_material_name);
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

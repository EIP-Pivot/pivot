#pragma once

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/DeletionQueue.hxx"
#include "pivot/graphics/DescriptorAllocator/DescriptorAllocator.hxx"
#include "pivot/graphics/DescriptorAllocator/DescriptorLayoutCache.hxx"
#include "pivot/graphics/PipelineStorage.hxx"
#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/VulkanRenderPass.hxx"
#include "pivot/graphics/VulkanSwapchain.hxx"
#include "pivot/graphics/interface/IRenderer.hxx"
#include "pivot/graphics/pivot.hxx"
#include "pivot/graphics/types/Frame.hxx"
#include "pivot/graphics/types/vk_types.hxx"
#include "pivot/graphics/vk_debug.hxx"

#include "pivot/graphics/DrawCallResolver.hxx"
#include "pivot/graphics/Renderer/CullingRenderer.hxx"
#include "pivot/graphics/Renderer/GraphicsRenderer.hxx"
#include "pivot/graphics/Renderer/ImGuiRenderer.hxx"

#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{
/// The validation layers used by the engine
const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

/// The device extensions required by the engine
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

/// @class VulkanApplication
/// @brief Main class of the graphics engine
///
/// To use it, you need to first load the textures and the 3D models using the appropriate methods
///
/// Then you will call the init() method, to initialise the Vulkan ressources
///
/// You can now call the draw() method when you are ready to render a new frame
class VulkanApplication : public VulkanBase
{
public:
    template <typename T>
    /// Alias for storing a Renderer and its associated command buffer
    requires std::is_base_of_v<IRenderer, T>
    using RendererStorage = std::vector<std::unique_ptr<T>>;

public:
    /// Default constructor
    VulkanApplication();
    /// Default destructor
    ~VulkanApplication();

    /// @brief Initialise the Vulkan ressources
    ///
    /// @throw VulkanException if something went awry
    void init();

    /// @brief draw the next frame
    ///
    /// @arg scene The 3D scene to draw
    /// @arg camera The information about the camera
    ///
    /// You must have already loaded your models and texture !
    void draw(DrawCallResolver::DrawSceneInformation sceneInformation, const CameraData &camera);

    /// @brief get Swapchain aspect ratio
    constexpr float getAspectRatio() const noexcept { return swapchain.getAspectRatio(); }

    /// Build the asset Storage
    void buildAssetStorage(AssetStorage::BuildFlags flags = AssetStorage::BuildFlagBits::eClear);

    template <validRenderer T>
    /// Add a Renderer to the frame
    T &addRenderer()
    {
        DEBUG_FUNCTION
        StorageUtils utils{
            .pipeline = pipelineStorage,
            .assets = assetStorage,
        };
        auto rendy = std::make_unique<T>(utils);
        auto &ret = *rendy;
        if constexpr (std::is_base_of_v<IGraphicsRenderer, T>) {
            graphicsRenderer.emplace_back(std::move(rendy));
        } else if constexpr (std::is_base_of_v<IComputeRenderer, T>) {
            computeRenderer.emplace_back(std::move(rendy));
        } else {
            throw std::logic_error("Unsuported Renderer : " + rendy->getType());
        }
        return ret;
    }

private:
    virtual void postInitialization();
    void recreateSwapchain();
    void initVulkanRessources();

    void createCommandPool();
    void createCommandBuffers();

    void createDepthResources();
    void createColorResources();
    void createRenderPass();
    void createFramebuffers();

public:
    /// The application asssets storage
    AssetStorage assetStorage;
    /// The application pipeline storage
    PipelineStorage pipelineStorage;

private:
    DescriptorAllocator descriptorAllocator;
    DescriptorLayoutCache layoutCache;

    vk::DescriptorPool imGuiPool = VK_NULL_HANDLE;
    vk::CommandPool commandPool = VK_NULL_HANDLE;

    RendererStorage<IGraphicsRenderer> graphicsRenderer;
    RendererStorage<IComputeRenderer> computeRenderer;

    DeletionQueue mainDeletionQueue;
    DeletionQueue swapchainDeletionQueue;
    VulkanSwapchain swapchain;

    uint8_t currentFrame = 0;
    std::array<Frame, MaxFrameInFlight> frames;

    VulkanRenderPass renderPass;

    AllocatedImage depthResources = {};
    AllocatedImage colorImage = {};

    std::vector<vk::Framebuffer> swapChainFramebuffers;
};

}    // namespace pivot::graphics

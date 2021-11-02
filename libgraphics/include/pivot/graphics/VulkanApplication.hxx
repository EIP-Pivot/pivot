#pragma once

#include <cstring>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/DeletionQueue.hxx"
#include "pivot/graphics/Swapchain.hxx"
#include "pivot/graphics/VulkanLoader.hxx"
#include "pivot/graphics/VulkanSwapchain.hxx"
#include "pivot/graphics/Window.hxx"
#include "pivot/graphics/common.hxx"
#include "pivot/graphics/types/Frame.hxx"
#include "pivot/graphics/types/Material.hxx"
#include "pivot/graphics/types/Mesh.hxx"
#include "pivot/graphics/types/RenderObject.hxx"
#include "pivot/graphics/types/vk_types.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics
{

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

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

class VulkanApplication : public VulkanLoader
{
private:
#ifdef NDEBUG
    bool bEnableValidationLayers = false;
#else
    bool bEnableValidationLayers = true;
#endif
    struct DrawBatch {
        std::string meshId;
        uint32_t first;
        uint32_t count;
    };

    struct SceneObjectsGPUData {
        std::vector<DrawBatch> objectDrawBatches;
        std::vector<gpuObject::UniformBufferObject> objectGPUData;
    };

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
    void draw(const std::vector<std::reference_wrapper<const RenderObject>> &sceneInformation,
              const gpuObject::CameraData &camera
#ifdef PIVOT_CULLING_DEBUG
              ,
              const std::optional<std::reference_wrapper<const gpuObject::CameraData>> cullingCamera = std::nullopt
#endif
    );

    /// @brief load the 3D models into CPU memory
    ///
    /// @arg paths the path for all individual file to load
    /// @return the number of file successfully loaded
    size_t load3DModels(const std::vector<std::filesystem::path> &paths);

    /// @brief load the Textures into CPU memory
    ///
    /// @arg paths the path for all individual file to load
    /// @return the number of file successfully loaded
    size_t loadTextures(const std::vector<std::filesystem::path> &);

    /// @brief get Swapchain aspect ratio
    float getAspectRatio() const;

    void recreateViewport(const vk::Extent2D &size);
    Swapchain &getViewportSwapchain() noexcept { return viewportContext.swapchain; }
    const vk::Sampler &getViewportSampler() const noexcept { return viewportContext.sampler; }
    uint32_t getCurrentFrame() const noexcept { return currentFrame; }

private:
    void pushModelsToGPU();
    void pushTexturesToGPU();

    template <vk_utils::is_copyable T>
    void copyBuffer(AllocatedBuffer &buffer, const std::vector<T> &data);
    template <vk_utils::is_copyable T>
    void copyBuffer(AllocatedBuffer &buffer, const T *data, size_t size);

    void immediateCommand(std::function<void(vk::CommandBuffer &)> &&function);
    void copyBufferToImage(const vk::Buffer &srcBuffer, vk::Image &dstImage, const vk::Extent3D &extent);
    void copyBufferToBuffer(const vk::Buffer &srcBuffer, vk::Buffer &dstBuffer, const vk::DeviceSize &size);
    void transitionImageLayout(vk::Image &image, vk::Format format, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout, uint32_t mipLevels = 1);
    void generateMipmaps(vk::Image &image, vk::Format imageFormat, vk::Extent3D size, uint32_t mipLevel);

    static bool checkValidationLayerSupport();
    static uint32_t debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);
    static vk::SampleCountFlagBits getMexUsableSampleCount(vk::PhysicalDevice &physical_device);
    static bool isDeviceSuitable(const vk::PhysicalDevice &gpu, const vk::SurfaceKHR &surface);
    static uint32_t rateDeviceSuitability(const vk::PhysicalDevice &device);
    static bool checkDeviceExtensionSupport(const vk::PhysicalDevice &device);

    SceneObjectsGPUData buildSceneObjectsGPUData(const std::vector<std::reference_wrapper<const RenderObject>> &objects,
                                                 const gpuObject::CameraData &camera);
    void buildIndirectBuffers(const std::vector<DrawBatch> &scene, Frame &frame);

    void postInitialization();
    void recreateSwapchain();
    void initVulkanRessources();

    AllocatedBuffer createBuffer(uint32_t allocSize, vk::BufferUsageFlags usage, vma::MemoryUsage memoryUsage);

    void createInstance();
    void createDebugMessenger();
    void createAllocator();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createUniformBuffers();
    void createSyncStructure();

    void createDescriptorSetLayout();
    void createTextureDescriptorSetLayout();

    void createDescriptorPool();
    void createDescriptorSets();
    void createTextureDescriptorSets();
    void createCommandPool();
    void createCommandBuffers();

    void createPipelineCache();
    void createPipelineLayout();
    void createPipeline();
    void createDepthResources();
    void createColorResources();
    void createRenderPass();
    void createIndirectBuffer();
    void createTextureSampler();
    void createFramebuffers();

    void createImGuiDescriptorPool();
    void initDearImGui();
    void createViewportRenderPass();
    void createViewportFramebuffers();
    void createImGuiSampler();
    void createViewportDepthResources();
    void createViewportColorResources();

public:
    /// The Window used to render 3D objects
    Window window;

    /// This will the store the textures, 3D models before behind uploaded to the GPU
    struct {
        std::unordered_map<std::string, std::vector<std::byte>> loadedTextures;
        std::unordered_map<std::string, vk::Extent3D> loadedTexturesSize;
        std::vector<Vertex> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
    } cpuStorage;
    /// Internal storage for the material
    MaterialStorage materials;
    /// Internal storage for the meshes
    MeshStorage loadedMeshes;
    /// Internal storage for the meshes' bounding boxes
    MeshBoundingBoxStorage meshesBoundingBoxes;
    /// Internal storage for the textures
    ImageStorage loadedTextures;

private:
    uint8_t currentFrame = 0;
    uint32_t mipLevels = 0;
    vk::SampleCountFlagBits maxMsaaSample = vk::SampleCountFlagBits::e1;

    AllocatedBuffer vertexBuffers{};
    AllocatedBuffer indicesBuffers{};

    struct {
        vk::Fence uploadFence = VK_NULL_HANDLE;
        vk::CommandPool commandPool = VK_NULL_HANDLE;
    } uploadContext = {};

    struct {
        vk::RenderPass renderPass = VK_NULL_HANDLE;
        std::vector<vk::Framebuffer> framebuffers;
        vk::CommandPool cmdPool = VK_NULL_HANDLE;
        std::vector<vk::CommandBuffer> cmdBuffer;
        vk::DescriptorPool pool = VK_NULL_HANDLE;
        vk::Sampler sampler = VK_NULL_HANDLE;
        Swapchain swapchain;
        AllocatedImage depthResources = {};
        AllocatedImage colorImage = {};
    } viewportContext = {};
    DeletionQueue viewportDeletionQueue;

    DeletionQueue mainDeletionQueue;
    DeletionQueue swapchainDeletionQueue;
    vk::DebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    vk::PhysicalDevice physical_device = VK_NULL_HANDLE;
    vma::Allocator allocator = VK_NULL_HANDLE;
    VulkanSwapchain swapchain;
    vk::SurfaceKHR surface = VK_NULL_HANDLE;

    //  Queues
    vk::Queue graphicsQueue = VK_NULL_HANDLE;
    vk::Queue presentQueue = VK_NULL_HANDLE;

    Frame frames[PIVOT_MAX_FRAME_FRAME_IN_FLIGHT];

    vk::RenderPass renderPass = VK_NULL_HANDLE;
    vk::DescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    vk::DescriptorSetLayout texturesSetLayout = VK_NULL_HANDLE;
    vk::DescriptorPool descriptorPool = VK_NULL_HANDLE;
    vk::DescriptorSet texturesSet = VK_NULL_HANDLE;

    vk::Sampler textureSampler = VK_NULL_HANDLE;

    vk::CommandPool commandPool = VK_NULL_HANDLE;

    std::vector<vk::CommandBuffer> commandBuffersPrimary;
    std::vector<vk::CommandBuffer> commandBuffersSecondary;

    AllocatedImage depthResources = {};
    AllocatedImage colorImage = {};

    vk::PipelineCache pipelineCache = VK_NULL_HANDLE;
    vk::PipelineLayout pipelineLayout = VK_NULL_HANDLE;
    vk::Pipeline graphicsPipeline = VK_NULL_HANDLE;

    std::vector<vk::Framebuffer> swapChainFramebuffers;
};

#ifndef VULKAN_APPLICATION_IMPLEMENTATION
#define VULKAN_APPLICATION_IMPLEMENTATION

template <vk_utils::is_copyable T>
void VulkanApplication::copyBuffer(AllocatedBuffer &buffer, const T *data, size_t size)
{
    void *mapped = nullptr;
    vmaMapMemory(allocator, buffer.memory, &mapped);
    std::memcpy(mapped, data, size);
    vmaUnmapMemory(allocator, buffer.memory);
}

template <vk_utils::is_copyable T>
void VulkanApplication::copyBuffer(AllocatedBuffer &buffer, const std::vector<T> &data)
{
    vk::DeviceSize size = sizeof(data[0]) * data.size();
    void *mapped = nullptr;
    vmaMapMemory(allocator, buffer.memory, &mapped);
    std::memcpy(mapped, data.data(), size);
    vmaUnmapMemory(allocator, buffer.memory);
}

#endif

}    // namespace pivot::graphics
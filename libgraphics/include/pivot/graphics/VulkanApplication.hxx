#pragma once

#include <cstring>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "pivot/graphics/Camera.hxx"
#include "pivot/graphics/DeletionQueue.hxx"
#include "pivot/graphics/Swapchain.hxx"
#include "pivot/graphics/VulkanLoader.hxx"
#include "pivot/graphics/interface/I3DScene.hxx"
#include "pivot/graphics/interface/IWindow.hxx"
#include "pivot/graphics/types/Frame.hxx"
#include "pivot/graphics/types/Material.hxx"
#include "pivot/graphics/types/Mesh.hxx"
#include "pivot/graphics/types/RenderObject.hxx"
#include "pivot/graphics/vk_utils.hxx"

#define MAX_OBJECT 1000
#define MAX_TEXTURES 1000

#define MAX_COMMANDS 100
#define MAX_MATERIALS 100

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

constexpr uint8_t MAX_FRAME_FRAME_IN_FLIGHT = 3;

class VulkanApplication : public VulkanLoader
{
public:
#ifdef NDEBUG
    const bool bEnableValidationLayers = false;
#else
    const bool bEnableValidationLayers = true;
#endif
    struct DrawBatch {
        std::string meshId;
        uint32_t first;
        uint32_t count;
    };

public:
    VulkanApplication();
    ~VulkanApplication();

    void init(IWindow &win);
    void draw(const I3DScene &scene, const Camera &camera, float fElapsedTime);

    size_t load3DModels(const std::vector<std::filesystem::path> &);
    size_t loadTexturess(const std::vector<std::filesystem::path> &);

private:
    void postInitialization();
    void recreateSwapchain();
    std::vector<DrawBatch> buildDrawBatch(std::vector<RenderObject> &object);
    void buildIndirectBuffers(const std::vector<DrawBatch> &scene, Frame &frame);

    void initVulkanRessources();

    template <vk_utils::is_copyable T>
    void copyBuffer(AllocatedBuffer &buffer, const std::vector<T> &data);
    template <vk_utils::is_copyable T>
    void copyBuffer(AllocatedBuffer &buffer, const T *data, size_t size);

    AllocatedBuffer createBuffer(uint32_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
    void immediateCommand(std::function<void(VkCommandBuffer &)> &&function);
    void copyBufferToImage(const VkBuffer &srcBuffer, VkImage &dstImage, const VkExtent3D &extent);
    void copyBufferToBuffer(const VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkDeviceSize &size);

    void transitionImageLayout(VkImage &image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels = 1);
    void generateMipmaps(VkImage &image, VkFormat imageFormat, VkExtent3D size, uint32_t mipLevel);

    void pushModelsToGPU();
    void pushTexturesToGPU();
    void createInstance();
    void createDebugMessenger();
    void createAllocator();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createUniformBuffers();
    void createSyncStructure();

    void createDescriptorSetsLayout();
    void createDescriptorPool();
    void createDescriptorSets();
    void createTextureDescriptorSets();
    void createCommandPool();
    void createCommandBuffers();
    void createPipeline();
    void createDepthResources();
    void createColorResources();
    void createRenderPass();
    void createIndirectBuffer();
    void createTextureSampler();
    void createFramebuffers();

private:
    bool checkDeviceExtensionSupport(const VkPhysicalDevice &device);
    bool isDeviceSuitable(const VkPhysicalDevice &gpu, const VkSurfaceKHR &surface);

    static uint32_t debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
                                  const VkDebugUtilsMessengerCallbackDataEXT *, void *);
    static bool checkValiationLayerSupport();

protected:
    std::optional<std::reference_wrapper<IWindow>> window;

    struct {
        std::unordered_map<std::string, std::vector<std::byte>> loadedTextures;
        std::unordered_map<std::string, VkExtent3D> loadedTexturesSize;
        std::vector<Vertex> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
    } cpuStorage;
    std::vector<gpuObject::Material> materials;
    std::unordered_map<std::string, GPUMesh> loadedMeshes;
    std::unordered_map<std::string, AllocatedImage> loadedTextures;

    uint32_t mipLevels = 0;
    // VkSampleCountFlagBits maxMsaaSample = VK_SAMPLE_COUNT_1_BIT;

    AllocatedBuffer vertexBuffers;
    AllocatedBuffer indicesBuffers;

    struct {
        VkFence uploadFence = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
    } uploadContext = {};

    DeletionQueue mainDeletionQueue;
    DeletionQueue swapchainDeletionQueue;
    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VmaAllocator allocator = VK_NULL_HANDLE;
    Swapchain swapchain;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    //  Queues
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    uint8_t currentFrame = 0;
    Frame frames[MAX_FRAME_FRAME_IN_FLIGHT];

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout texturesSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet texturesSet = VK_NULL_HANDLE;

    VkSampler textureSampler = VK_NULL_HANDLE;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;

    AllocatedImage depthResources = {};
    AllocatedImage colorImage = {};

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> swapChainFramebuffers;
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
    VkDeviceSize size = sizeof(data[0]) * data.size();
    void *mapped = nullptr;
    vmaMapMemory(allocator, buffer.memory, &mapped);
    std::memcpy(mapped, data.data(), size);
    vmaUnmapMemory(allocator, buffer.memory);
}

#endif
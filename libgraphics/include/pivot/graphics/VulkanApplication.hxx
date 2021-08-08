#pragma once

#include <cstring>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "pivot/graphics/DeletionQueue.hxx"
#include "pivot/graphics/Swapchain.hxx"
#include "pivot/graphics/VulkanLoader.hxx"
#include "pivot/graphics/interface/I3DScene.hxx"
#include "pivot/graphics/interface/IWindow.hxx"
#include "pivot/graphics/types/Mesh.hxx"

#include <Logger.hpp>

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

constexpr uint8_t MAX_FRAME_FRAME_IN_FLIGHT = 2;

class VulkanApplication : public VulkanLoader
{
public:
#ifdef NDEBUG
    const bool bEnableValidationLayers = false;
#else
    const bool bEnableValidationLayers = true;
#endif

public:
    VulkanApplication();
    ~VulkanApplication();

    void initBaseRessources();
    size_t load3DModels(const std::vector<std::filesystem::path> &);
    size_t loadTexturess(const std::vector<std::filesystem::path> &);

    void setActiveScene(I3DScene &scene) { activeScene = scene; }

protected:
    template <typename T>
    void copyBuffer(AllocatedBuffer &buffer, const std::vector<T> &data);
    template <typename T>
    void copyBuffer(AllocatedBuffer &buffer, const T *data, size_t size);

    void copyBuffer(const VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkDeviceSize &size);
    AllocatedBuffer createBuffer(uint32_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
    void immediateCommand(std::function<void(VkCommandBuffer &)> &&function);
    void copyBufferToImage(const VkBuffer &srcBuffer, VkImage &dstImage, const VkExtent3D &extent);
    void transitionImageLayout(VkImage &image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels = 1);
    void generateMipmaps(VkImage &image, VkFormat imageFormat, VkExtent3D size, uint32_t mipLevel);

    void initVisualRessources();

    void pushModelsToGPU();
    void pushTexturesToGPU();

private:
    void createInstance();
    void createDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createAllocator();
    void createQueue();
    void createSurface();

private:
    bool checkDeviceExtensionSupport(const VkPhysicalDevice &device);
    bool isDeviceSuitable(const VkPhysicalDevice &gpu, const VkSurfaceKHR &surface);

    static uint32_t debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
                                  const VkDebugUtilsMessengerCallbackDataEXT *, void *);
    static bool checkValiationLayerSupport();

private:
    uint32_t mipLevels = 0;
    // VkSampleCountFlagBits maxMsaaSample = VK_SAMPLE_COUNT_1_BIT;

    std::optional<std::reference_wrapper<IWindow>> window;
    std::optional<std::reference_wrapper<I3DScene>> activeScene;

    struct {
        std::unordered_map<std::string, std::vector<std::byte>> loadedTextures;
        std::unordered_map<std::string, VkExtent3D> loadedTexturesSize;
        std::vector<Vertex> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
    } cpuStorage;
    std::unordered_map<std::string, GPUMesh> loadedMeshes;
    std::unordered_map<std::string, AllocatedImage> loadedTextures;
    AllocatedBuffer vertexBuffers;
    AllocatedBuffer indicesBuffers;

    struct {
        VkQueue uploadQueue = VK_NULL_HANDLE;
        VkFence uploadFence = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
    } uploadContext = {};
    DeletionQueue mainDeletionQueue;
    DeletionQueue swapchainDeletionQueue;

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VmaAllocator allocator = VK_NULL_HANDLE;
    Swapchain swapchain;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    //  Queues
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
};

#ifndef VULKAN_APPLICATION_IMPLEMENTATION
#define VULKAN_APPLICATION_IMPLEMENTATION

template <typename T>
void VulkanApplication::copyBuffer(AllocatedBuffer &buffer, const T *data, size_t size)
{
    void *mapped = nullptr;
    vmaMapMemory(allocator, buffer.memory, &mapped);
    std::memcpy(mapped, data, size);
    vmaUnmapMemory(allocator, buffer.memory);
}

template <typename T>
void VulkanApplication::copyBuffer(AllocatedBuffer &buffer, const std::vector<T> &data)
{
    VkDeviceSize size = sizeof(data[0]) * data.size();
    void *mapped = nullptr;
    vmaMapMemory(allocator, buffer.memory, &mapped);
    std::memcpy(mapped, data.data(), size);
    vmaUnmapMemory(allocator, buffer.memory);
}

#endif
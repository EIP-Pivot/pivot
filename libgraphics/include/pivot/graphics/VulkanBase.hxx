#pragma once

#include "pivot/graphics/DeletionQueue.hxx"
#include "pivot/graphics/VulkanLoader.hxx"
#include "pivot/graphics/VulkanSwapchain.hxx"
#include "pivot/graphics/Window.hxx"
#include "pivot/graphics/common.hxx"
#include "pivot/graphics/types/Frame.hxx"

#include <vk_mem_alloc.hpp>

namespace pivot::graphics
{

/// The validations layers to activate
const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

/// The device extensions pivot is using
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

/// The instance extensions pivot is using
const std::vector<const char *> instanceExtensions = {

};

/// @class VulkanBase
/// This class hold the bare minium of Vulkan ressources
class VulkanBase : public VulkanLoader
{
public:
    /// Whether or not the validation layers are activated
    /// Is true by default when compiling with debug enabled
#ifdef NDEBUG
    bool bEnableValidationLayers = false;
#else
    bool bEnableValidationLayers = true;
#endif

protected:
    /// Constructor
    VulkanBase();
    /// Destructor
    ~VulkanBase();

    /// Initialize the ressources
    void init();
    /// Destroy the ressources
    void destroy();

private:
    static bool checkValidationLayerSupport();
    static uint32_t debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);
    static bool isDeviceSuitable(const vk::PhysicalDevice &gpu, const vk::SurfaceKHR &surface);
    static uint32_t rateDeviceSuitability(const vk::PhysicalDevice &device);
    static bool checkDeviceExtensionSupport(const vk::PhysicalDevice &device);

private:
    void createInstance();
    void createDebugMessenger();
    void createAllocator();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncStructure();

protected:
    /// The Window used to render 3D objects
    Window window;

    /// The queue used for rendering
    vk::Queue graphicsQueue = VK_NULL_HANDLE;
    /// The queue used for presenting
    vk::Queue presentQueue = VK_NULL_HANDLE;

    /// @cond
    vk::DebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    vk::PhysicalDevice physical_device = VK_NULL_HANDLE;
    vma::Allocator allocator = VK_NULL_HANDLE;
    vk::SurfaceKHR surface = VK_NULL_HANDLE;

    VulkanSwapchain swapchain;

    vk::CommandPool primaryCommandPool = VK_NULL_HANDLE;
    std::vector<vk::CommandBuffer> commandBuffer;

    Frame framesSync[PIVOT_MAX_FRAME_FRAME_IN_FLIGHT];

    /// @endcond
private:
    DeletionQueue baseDeletionQueue;
};

}    // namespace pivot::graphics
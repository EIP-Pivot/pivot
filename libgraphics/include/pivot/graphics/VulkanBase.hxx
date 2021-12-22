#pragma once

#include "pivot/graphics/DeletionQueue.hxx"
#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/VulkanLoader.hxx"
#include "pivot/graphics/Window.hxx"
#include "pivot/graphics/abstract/AImmediateCommand.hxx"

namespace pivot::graphics
{

/// @class VulkanBase
/// Handle the bare minimum of Vulkan ressources to perform GPU manipulation
class VulkanBase : public VulkanLoader, public abstract::AImmediateCommand
{
public:
    /// Default ctor
    VulkanBase(const std::string &windowName = "VulkanBase");
    /// Default ctor
    ~VulkanBase();

    /// Initialize the ressources
    void init(const std::vector<const char *> &instanceExtensions, const std::vector<const char *> &deviceExtensions,
              const std::vector<const char *> &validationLayers);

    /// Flush the deletionQueue
    void destroy();

    /// Check if validation layer are supported
    static bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);

private:
    static uint32_t debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);
    static bool isDeviceSuitable(const vk::PhysicalDevice &gpu, const vk::SurfaceKHR &surface,
                                 const std::vector<const char *> &deviceExtensions);
    static uint32_t rateDeviceSuitability(const vk::PhysicalDevice &device);
    static bool checkDeviceExtensionSupport(const vk::PhysicalDevice &device,
                                            const std::vector<const char *> &deviceExtensions);

private:
    void createInstance(const std::vector<const char *> &instanceExtensions,
                        const std::vector<const char *> &validationLayers);
    void createDebugMessenger();
    void createSurface();
    void selectPhysicalDevice(const std::vector<const char *> &deviceExtensions);
    void createLogicalDevice(const std::vector<const char *> &deviceExtensions);
    void createAllocator();

public:
    /// The Window used to render 3D objects
    Window window;
    /// The Surface used by Vulkan to draw onto
    vk::SurfaceKHR surface = VK_NULL_HANDLE;
    /// The selected GPU used by Vulkan
    vk::PhysicalDevice physical_device = VK_NULL_HANDLE;
    /// A VulkanMemoryAllocator
    vma::Allocator allocator = VK_NULL_HANDLE;

protected:
    /// Maximum mipmap level
    uint32_t mipLevels = 0;
    /// Maximum support msaaSample value.
    vk::SampleCountFlagBits maxMsaaSample = vk::SampleCountFlagBits::e1;
    /// List of GPU features
    vk::PhysicalDeviceFeatures deviceFeature{};
    /// Indices of the selected queues
    QueueFamilyIndices queueIndices;

    /// The queue used for graphics processing
    vk::Queue graphicsQueue = VK_NULL_HANDLE;
    /// The queue used for presenting, can be the same as the graphics queue
    vk::Queue presentQueue = VK_NULL_HANDLE;

private:
    vk::DebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    DeletionQueue baseDeletionQueue;

#ifdef NDEBUG
    bool bEnableValidationLayers = false;
#else
    bool bEnableValidationLayers = true;
#endif
};

}    // namespace pivot::graphics
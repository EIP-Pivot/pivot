#pragma once

#include <vulkan/vulkan.hpp>

/// @class VulkanLoader
///
/// @brief A class to abstract dynamic lookout of Vulkan function
///
/// Make this class a parent of the main Vulkan class, and call the methods as soon as possible, to automagically setup
/// Vulkan dispatcher.
class VulkanLoader
{
protected:
    /// Default constructor
    VulkanLoader();

    /// Create a vulkan instance, and use it to load symbols
    void createInstance(vk::InstanceCreateInfo &info);
    /// Create a logical device, and use it to load symbols
    void createLogicalDevice(vk::PhysicalDevice &gpu, vk::DeviceCreateInfo &info);

public:
    /// @cond
    vk::Instance instance = VK_NULL_HANDLE;
    vk::Device device = VK_NULL_HANDLE;
    /// @endcond

protected:
    vk::DynamicLoader loader;
};

#include "pivot/graphics/VulkanLoader.hxx"

#include "pivot/pivot.hxx"

namespace pivot::graphics
{
VulkanLoader::VulkanLoader()
{
    DEBUG_FUNCTION();
    auto vkGetInstanceProcAddr = loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
}

void VulkanLoader::createInstance(vk::InstanceCreateInfo &info)
{
    DEBUG_FUNCTION();
    instance = vk::createInstance(info);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
}

void VulkanLoader::createLogicalDevice(vk::PhysicalDevice &gpu, vk::DeviceCreateInfo &info)
{
    DEBUG_FUNCTION();
    device = gpu.createDevice(info);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
}
}    // namespace pivot::graphics

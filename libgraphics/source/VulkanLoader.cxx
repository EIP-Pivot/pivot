#include "pivot/graphics/VulkanLoader.hxx"

namespace pivot::graphics
{
VulkanLoader::VulkanLoader()
{
    auto vkGetInstanceProcAddr = loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
}

void VulkanLoader::createInstance(vk::InstanceCreateInfo &info)
{
    instance = vk::createInstance(info);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
}

void VulkanLoader::createLogicalDevice(vk::PhysicalDevice &gpu, vk::DeviceCreateInfo &info)
{
    device = gpu.createDevice(info);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
}
}    // namespace pivot::graphics

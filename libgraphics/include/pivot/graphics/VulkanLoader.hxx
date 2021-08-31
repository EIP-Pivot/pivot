#pragma once

#include <vulkan/vulkan.hpp>

class VulkanLoader
{
protected:
    VulkanLoader();

    void createInstance(vk::InstanceCreateInfo &info);
    void createLogicalDevice(vk::PhysicalDevice &gpu, vk::DeviceCreateInfo &info);

protected:
    vk::DynamicLoader loader;
    vk::Instance instance;
    vk::Device device;
};

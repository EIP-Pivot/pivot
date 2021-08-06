#pragma once

#include "assert.h"
#include <cstdint>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    inline bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    static QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &device, const VkSurfaceKHR &surface)
    {
        QueueFamilyIndices indices;
        uint32_t uFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &uFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> family_property_list(uFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &uFamilyCount, family_property_list.data());

        for (uint32_t i = 0; i < uFamilyCount; ++i) {
            if (family_property_list.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) indices.presentFamily = i;
        }
        return indices;
    }
};

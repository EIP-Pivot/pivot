#pragma once

#include "assert.h"
#include <cstdint>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    constexpr bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    static QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface)
    {
        QueueFamilyIndices indices;
        auto family_property_list = device.getQueueFamilyProperties();

        for (uint32_t i = 0; i < family_property_list.size() && !indices.isComplete(); ++i) {
            if (family_property_list.at(i).queueFlags & vk::QueueFlagBits::eGraphics) indices.graphicsFamily = i;
            if (device.getSurfaceSupportKHR(i, surface)) indices.presentFamily = i;
        }
        return indices;
    }
};

#pragma once

#include "assert.h"
#include <cstdint>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{
/// @struct QueueFamilyIndices
///
/// @brief Utility struct to find viable Vulkan queue
struct QueueFamilyIndices {
    /// Index of a potential graphics queue
    std::optional<uint32_t> graphicsFamily;
    /// Index of a potential presentation queue
    std::optional<uint32_t> presentFamily;
    /// Index of a potential transfer queue
    std::optional<uint32_t> transferFamily;

    /// Has the two types of queue been found ?
    /// @return true if both types of queue are found
    constexpr bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
    }

    /// Search for viable queues on the physical device
    ///
    /// @param device Physical Device to search for queues
    /// @param surface Vulkan surface that we be using the present queue
    static QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface)
    {
        QueueFamilyIndices indices;
        auto family_property_list = device.getQueueFamilyProperties();
        for (uint32_t i = 0; i < family_property_list.size() && !indices.isComplete(); ++i) {
            if (family_property_list.at(i).queueFlags & vk::QueueFlagBits::eGraphics) indices.graphicsFamily = i;
            if (family_property_list.at(i).queueFlags & vk::QueueFlagBits::eTransfer &&
                family_property_list.at(i).queueFlags & vk::QueueFlagBits::eGraphics)
                indices.transferFamily = i;

            if (device.getSurfaceSupportKHR(i, surface)) indices.presentFamily = i;
        }
        return indices;
    }
};
}    // namespace pivot::graphics

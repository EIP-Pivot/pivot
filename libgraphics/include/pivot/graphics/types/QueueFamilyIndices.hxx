#pragma once

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
    /// Index of a potential compute queue
    std::optional<uint32_t> computeFamilty;

    /// Has the two types of queue been found ?
    /// @return true if both types of queue are found
    constexpr bool isComplete() const noexcept
    {
        return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value() &&
               computeFamilty.has_value();
    }

    /// Search for viable queues on the physical device
    ///
    /// @param device Physical Device to search for queues
    /// @param surface Vulkan surface that we be using the present queue
    static QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);
};
}    // namespace pivot::graphics

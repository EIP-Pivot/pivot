#pragma once

#include <cstdint>
#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{
/// @struct QueueFamilyIndices
///
/// @brief Utility struct to find viable Vulkan queue
struct QueueFamilyIndices {
    /// Index of a potential graphics queue
    std::optional<std::uint32_t> graphicsFamily;
    /// Index of a potential presentation queue
    std::optional<std::uint32_t> presentFamily;
    /// Index of a potential transfer queue
    std::optional<std::uint32_t> transferFamily;
    /// Index of a potential compute queue
    std::optional<std::uint32_t> computeFamily;

    /// Has the two types of queue been found ?
    /// @return true if both types of queue are found
    constexpr bool isComplete() const noexcept
    {
        return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value() &&
               computeFamily.has_value();
    }

    std::pair<vk::QueueFlags, std::set<std::uint32_t>> getUniqueQueues() const noexcept
    {
        std::set<std::uint32_t> set;
        vk::QueueFlags flag;
        if (graphicsFamily.has_value()) {
            const auto &[_, inserted] = set.insert(graphicsFamily.value());
            if (inserted) flag |= vk::QueueFlagBits::eGraphics;
        }
        if (presentFamily.has_value()) {
            const auto &[_, inserted] = set.insert(presentFamily.value());
            if (inserted) flag |= vk::QueueFlagBits::eGraphics;
        }
        if (transferFamily.has_value()) {
            const auto &[_, inserted] = set.insert(transferFamily.value());
            if (inserted) flag |= vk::QueueFlagBits::eTransfer;
        }
        if (computeFamily.has_value()) {
            const auto &[_, inserted] = set.insert(computeFamily.value());
            if (inserted) flag |= vk::QueueFlagBits::eCompute;
        }
        return {flag, set};
    }

    /// Search for viable queues on the physical device
    ///
    /// @param device Physical Device to search for queues
    /// @param surface Vulkan surface that we be using the present queue
    static QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);
};
}    // namespace pivot::graphics

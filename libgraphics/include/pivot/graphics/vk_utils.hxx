#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "pivot/graphics/VulkanException.hxx"

#define VK_TRY(x)                                \
    {                                            \
        VkResult err = x;                        \
        if (err) { throw VulkanException(err); } \
    }

namespace vk_utils
{
std::vector<std::byte> readFile(const std::string &filename);
VkShaderModule createShaderModule(const VkDevice &device, const std::vector<std::byte> &code);
VkFormat findSupportedFormat(VkPhysicalDevice &gpu, const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                             VkFormatFeatureFlags features);
bool hasStencilComponent(VkFormat format);
uint32_t findMemoryType(VkPhysicalDevice &physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

template <class... FailedValue>
bool isSwapchainInvalid(const VkResult result, const FailedValue... failedResult)
{
    if (((result == failedResult) || ...)) {
        return true;
    } else {
        VK_TRY(result);
        return false;
    }
}
std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance &instance);

namespace tools
{
    const std::string to_string(VkSampleCountFlagBits count);
    const std::string to_string(VkCullModeFlags count);

    std::string physicalDeviceTypeString(VkPhysicalDeviceType type);
}    // namespace tools
}    // namespace vk_utils

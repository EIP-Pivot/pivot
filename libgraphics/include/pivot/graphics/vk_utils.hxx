#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/VulkanException.hxx"

#define VK_TRY(x)                                                       \
    {                                                                   \
        vk::Result err = x;                                             \
        if (err < vk::Result::eSuccess) { throw VulkanException(err); } \
    }

namespace vk_utils
{
template <typename T>
concept is_copyable = requires
{
    std::is_standard_layout_v<T>;
    typename std::vector<T>;
};

void vk_try(vk::Result res);
void vk_try(VkResult res);
template <class... FailedValue>
bool vk_try_mutiple(const vk::Result result, const FailedValue... failedResult)
{
    if (((result == failedResult) || ...)) {
        return true;
    } else {
        VK_TRY(result);
        return false;
    }
}

std::vector<std::byte> readFile(const std::string &filename);
vk::ShaderModule createShaderModule(const vk::Device &device, const std::vector<std::byte> &code);
vk::Format findSupportedFormat(vk::PhysicalDevice &gpu, const std::vector<vk::Format> &candidates,
                               vk::ImageTiling tiling, vk::FormatFeatureFlags features);
bool hasStencilComponent(vk::Format format) noexcept;

namespace tools
{
    const std::string to_string(vk::SampleCountFlagBits count) noexcept;
    const std::string to_string(vk::CullModeFlagBits count) noexcept;

    std::string physicalDeviceTypeString(vk::PhysicalDeviceType type) noexcept;
}    // namespace tools
}    // namespace vk_utils

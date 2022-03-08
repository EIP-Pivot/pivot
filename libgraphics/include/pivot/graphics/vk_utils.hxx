#pragma once

#include <Logger.hpp>
#include <cstddef>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/VulkanException.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

namespace pivot::graphics
{

namespace vk_utils
{
    template <typename T>
    concept is_copyable = requires
    {
        std::is_standard_layout_v<T>;
        typename std::vector<T>;
    };

    constexpr void vk_try(vk::Result err)
    {
        if (err < vk::Result::eSuccess) throw VulkanException(err);
    }

    constexpr void vk_try(VkResult res) { vk_try(vk::Result(res)); }

    template <class... FailedValue>
    constexpr bool vk_try_mutiple(const vk::Result result, const FailedValue... failedResult)
    {
        if (((result == failedResult) || ...)) {
            return true;
        } else {
            vk_try(result);
            return false;
        }
    }

    std::vector<std::byte> readFile(const std::string &filename);
    vk::ShaderModule createShaderModule(const vk::Device &device, const std::vector<std::byte> &code);

    vk::SampleCountFlagBits getMaxUsableSampleCount(vk::PhysicalDevice &physical_device);
    vk::Format findSupportedFormat(vk::PhysicalDevice &gpu, const std::vector<vk::Format> &candidates,
                                   vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    bool hasStencilComponent(vk::Format format) noexcept;

    namespace tools
    {
        template <typename T>
        void print_array(Logger::Stream stream, const std::string_view &message, const std::vector<T> &val)
        {
            stream << message << ": [";
            for (const auto &i: val) {
                stream << i;
                if (i != val.back()) stream << ", ";
            }
            stream << "]";
        }
        std::string bytesToString(uint64_t bytes);
        const std::string to_string(vk::SampleCountFlagBits count) noexcept;
        const std::string to_string(vk::CullModeFlagBits count) noexcept;

        std::string physicalDeviceTypeString(vk::PhysicalDeviceType type) noexcept;
    }    // namespace tools
}    // namespace vk_utils
}    // namespace pivot::graphics

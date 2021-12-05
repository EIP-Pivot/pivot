#pragma once

#include "pivot/graphics/VulkanException.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

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

template <vk_utils::is_copyable T>
void copyBuffer(vma::Allocator &allocator, AllocatedBuffer &buffer, const T *data, size_t size)
{
    assert(data);
    void *mapped = allocator.mapMemory(buffer.memory);
    std::memcpy(mapped, data, size);
    allocator.unmapMemory(buffer.memory);
}

template <vk_utils::is_copyable T>
void copyBuffer(vma::Allocator &allocator, AllocatedBuffer &buffer, const std::vector<T> &data)
{
    if (data.empty()) return;
    vk::DeviceSize size = sizeof(data.at(0)) * data.size();
    void *mapped = allocator.mapMemory(buffer.memory);
    std::memcpy(mapped, data.data(), size);
    allocator.unmapMemory(buffer.memory);
}

vk::SampleCountFlagBits getMexUsableSampleCount(vk::PhysicalDevice &physical_device);

AllocatedBuffer createBuffer(vma::Allocator &allocator, uint32_t allocSize, vk::BufferUsageFlags usage,
                             vma::MemoryUsage memoryUsage);

constexpr void vk_try(vk::Result res) { VK_TRY(res); }

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

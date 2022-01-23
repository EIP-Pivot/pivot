#pragma once

#include <Logger.hpp>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/VulkanException.hxx"
#include "pivot/graphics/abstract/AImmediateCommand.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

namespace pivot::graphics::vk_utils
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

AllocatedBuffer createBuffer(vma::Allocator &allocator, uint32_t allocSize, vk::BufferUsageFlags usage,
                             vma::MemoryUsage memoryUsage);

template <pivot::graphics::vk_utils::is_copyable T>
void copyBuffer(vma::Allocator &allocator, AllocatedBuffer &buffer, const T *data, size_t size)
{
    void *mapped = allocator.mapMemory(buffer.memory);
    std::memcpy(mapped, data, size);
    allocator.unmapMemory(buffer.memory);
}

template <pivot::graphics::vk_utils::is_copyable T>
void copyBuffer(vma::Allocator &allocator, AllocatedBuffer &buffer, const std::vector<T> &data)
{
    vk::DeviceSize size = sizeof(T) * data.size();
    void *mapped = allocator.mapMemory(buffer.memory);
    std::memcpy(mapped, data.data(), size);
    allocator.unmapMemory(buffer.memory);
}

AllocatedBuffer createBuffer(vma::Allocator &allocator, uint32_t allocSize, vk::BufferUsageFlags usage,
                             vma::MemoryUsage memoryUsage);
void copyBufferToBuffer(abstract::AImmediateCommand &, const vk::Buffer &srcBuffer, vk::Buffer &dstBuffer,
                        const vk::DeviceSize &size);
void copyBufferToImage(abstract::AImmediateCommand &, const vk::Buffer &srcBuffer, vk::Image &dstImage,
                       const vk::Extent3D &extent);
void transitionImageLayout(abstract::AImmediateCommand &, vk::Image &image, vk::Format format,
                           vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels = 1);
void generateMipmaps(VulkanBase &, vk::Image &image, vk::Format imageFormat, vk::Extent3D size, uint32_t mipLevel);
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

    const std::string to_string(vk::SampleCountFlagBits count) noexcept;
    const std::string to_string(vk::CullModeFlagBits count) noexcept;

    std::string physicalDeviceTypeString(vk::PhysicalDeviceType type) noexcept;
}    // namespace tools
}    // namespace pivot::graphics::vk_utils

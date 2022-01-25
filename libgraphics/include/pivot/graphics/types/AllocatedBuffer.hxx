#pragma once

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics
{

class AllocatedImage;

/// @class AllocatedBuffer
///
/// @brief Utility class to keep track of a Vulkan buffer and its allocated memory
class AllocatedBuffer
{
public:
    AllocatedBuffer();
    ~AllocatedBuffer();

    AllocatedBuffer cloneBuffer(VulkanBase &i, const vk::BufferUsageFlags usage, const vma::MemoryUsage memoryUsage);

    template <vk_utils::is_copyable T>
    void copyBuffer(vma::Allocator &allocator, const T *data, size_t size)
    {
        void *mapped = allocator.mapMemory(memory);
        std::memcpy(mapped, data, size);
        allocator.unmapMemory(memory);
    }

    template <vk_utils::is_copyable T>
    void copyBuffer(vma::Allocator &allocator, const std::vector<T> &data)
    {
        void *mapped = allocator.mapMemory(memory);
        std::memcpy(mapped, data.data(), sizeof(T) * data.size());
        allocator.unmapMemory(memory);
    }

    void copyToImage(abstract::AImmediateCommand &, AllocatedImage &dstImage) const;
    operator bool() const noexcept;

public:
    static AllocatedBuffer create(VulkanBase &base, uint32_t allocSize, vk::BufferUsageFlags usage,
                                  vma::MemoryUsage memoryUsage);
    static void destroy(VulkanBase &base, AllocatedBuffer &buffer);

public:
    //// @cond
    vk::Buffer buffer = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
    std::uint32_t size = 0;

    //// @endcond
};

}    // namespace pivot::graphics
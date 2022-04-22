#pragma once

#include <span>
#include <string>
#include <vector>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"
#include "pivot/graphics/types/common.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics
{

/// @class VulkanAllocator
/// @brief Wrapper around VMA
///
/// This class store the vma::Allocator object and expose it with simplified function to use it. It also allow us to
/// monitor its usage
class VulkanAllocator
{
public:
    /// Hold the memory usage of the allocator
    struct GPUMemoryStats {
        /// The amount of used bytes
        uint64_t used = 0;
        /// the amount of available bytes
        uint64_t free = 0;
    };

public:
    /// Constructor
    VulkanAllocator();
    /// Destructor
    ~VulkanAllocator();

    /// Create the vma::Allocator object
    void init(const vma::AllocatorCreateInfo &info);
    /// Destory the vma::Allocator object. Does not free any allocated memory
    void destroy() { allocator.destroy(); }

    template <typename T>
    /// Create a buffer.
    AllocatedBuffer<T> createBuffer(std::size_t size, vk::BufferUsageFlags usage, vma::MemoryUsage memoryUsage,
                                    vma::AllocationCreateFlags flags = {})
    {
        assert(size != 0);
        AllocatedBuffer<T> buffer{
            .size = size,
            .flags = flags,
        };
        vk::BufferCreateInfo bufferInfo{
            .size = buffer.getBytesSize(),
            .usage = usage,
        };
        vma::AllocationCreateInfo vmaallocInfo;
        vmaallocInfo.usage = memoryUsage;
        vmaallocInfo.flags = flags;
        std::tie(buffer.buffer, buffer.memory) = allocator.createBuffer(bufferInfo, vmaallocInfo, buffer.info);
        return buffer;
    }

    /// @brief Create an image.
    ///
    /// The layout is undefined
    AllocatedImage createImage(const vk::ImageCreateInfo &info, const vma::AllocationCreateInfo &allocInfo);

    template <typename T>
    /// Map buffer memory to a pointer
    T *mapMemory(AllocatedBuffer<T> &buffer)
    {
        assert(buffer);
        return static_cast<T *>(allocator.mapMemory(buffer.memory));
    }

    template <typename T>
    /// Map buffer memory as a read-only pointer
    const T *const mapMemory(AllocatedBuffer<T> &buffer) const
    {
        assert(buffer);
        return static_cast<const T *const>(allocator.mapMemory(buffer.memory));
    }

    template <typename T>
    /// @brief Unmap buffer memory.
    ///
    /// It must be called the same number of times mapMemory() is called.
    /// It is safe to call even if the buffer has been created with vma::AllocationCreateFlagBits::eMapped.
    void unmapMemory(AllocatedBuffer<T> &buffer)
    {
        assert(buffer);
        allocator.unmapMemory(buffer.memory);
    }

    template <typename T>
    /// Copy the data into a buffer
    requires std::is_standard_layout_v<T>
    void copyBuffer(AllocatedBuffer<T> &buffer, const T *data, size_t size)
    {
        assert(buffer);
        assert(buffer.getBytesSize() >= size);
        if (size == 0) return;
        auto *mapped = mapMemory<T>(buffer);
        std::memcpy(mapped, data, buffer.getBytesSize());
        unmapMemory(buffer);
    }

    template <typename T>
    /// Copy the vector into the buffer
    requires std::is_standard_layout_v<T>
    void copyBuffer(AllocatedBuffer<T> &buffer, const std::span<T> &data)
    {
        return copyBuffer(buffer, data.data(), data.size_bytes());
    }

    template <typename T>
    /// Destroy an allocated buffer
    void destroyBuffer(AllocatedBuffer<T> &buffer)
    {
        allocator.destroyBuffer(buffer.buffer, buffer.memory);
    }
    /// Destroy an image. Does not destroy its image view
    void destroyImage(AllocatedImage &image) { allocator.destroyImage(image.image, image.memory); }

    /// Print memory status to the logger
    void dumpStats();
    /// Return the current status of the memory
    GPUMemoryStats getStats();

private:
    vma::Allocator allocator;
    vk::PhysicalDeviceMemoryProperties properties;
};

}    // namespace pivot::graphics

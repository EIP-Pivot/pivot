#pragma once

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

    /// Create a buffer.
    AllocatedBuffer createBuffer(const vk::DeviceSize &allocSize, const vk::BufferUsageFlags &usage,
                                 const vma::MemoryUsage &memoryUsage, const vma::AllocationCreateFlags &flags = {});
    /// @brief Create an image.
    ///
    /// The layout is undefined
    AllocatedImage createImage(const vk::ImageCreateInfo &info, const vma::AllocationCreateInfo &allocInfo);

    template <typename T>
    /// Map buffer memory to a pointer
    T *mapMemory(AllocatedBuffer &buffer)
    {
        return static_cast<T *>(allocator.mapMemory(buffer.memory));
    }

    template <typename T>
    /// Map buffer memory as a read-only pointer
    const T *const mapMemory(AllocatedBuffer &buffer) const
    {
        return static_cast<const T *const>(allocator.mapMemory(buffer.memory));
    }

    /// @brief Unmap buffer memory.
    ///
    /// It must be called the same number of times mapMemory() is called.
    /// It is safe to call if the buffer has been created with vma::AllocationCreateFlagBits::eMapped.
    void unmapMemory(AllocatedBuffer &buffer) { allocator.unmapMemory(buffer.memory); }

    template <vk_utils::is_copyable T>
    /// Copy the data into a buffer
    void copyBuffer(AllocatedBuffer &buffer, const T *data, size_t size)
    {
        auto *mapped = mapMemory<T>(buffer);
        std::memcpy(mapped, data, size);
        unmapMemory(buffer);
    }

    template <vk_utils::is_copyable T>
    /// Copy the vector into the buffer
    void copyBuffer(AllocatedBuffer &buffer, const std::vector<T> &data)
    {
        auto *mapped = mapMemory<T>(buffer);
        std::memcpy(mapped, data.data(), sizeof(T) * data.size());
        unmapMemory(buffer);
    }

    /// Destroy an allocated buffer
    void destroyBuffer(AllocatedBuffer &buffer) { allocator.destroyBuffer(buffer.buffer, buffer.memory); }
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

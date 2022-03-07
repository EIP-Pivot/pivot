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

class VulkanAllocator
{
public:
    struct GPUMemoryStats {
        uint64_t used = 0;
        uint64_t free = 0;
    };

public:
    VulkanAllocator();
    ~VulkanAllocator();

    void init(const vma::AllocatorCreateInfo &info);

    AllocatedBuffer createBuffer(const vk::DeviceSize &allocSize, const vk::BufferUsageFlags &usage,
                                 const vma::MemoryUsage &memoryUsage, const vma::AllocationCreateFlags &flags = {});
    AllocatedImage createImage(const vk::ImageCreateInfo &info, const vma::AllocationCreateInfo &allocInfo);

    template <typename T>
    T *mapMemory(AllocatedBuffer &buffer)
    {
        return static_cast<T *>(allocator.mapMemory(buffer.memory));
    }

    template <typename T>
    const T *const mapMemory(AllocatedBuffer &buffer) const
    {
        return static_cast<const T *const>(allocator.mapMemory(buffer.memory));
    }

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

    void destroyBuffer(AllocatedBuffer &buffer) { allocator.destroyBuffer(buffer.buffer, buffer.memory); }
    void destroyImage(AllocatedImage &image) { allocator.destroyImage(image.image, image.memory); }
    void destroy(AllocatedBuffer &buffer) { destroyBuffer(buffer); }
    void destroy(AllocatedImage &image) { destroyImage(image); }
    void destroy() { allocator.destroy(); }

    void dumpStats();
    GPUMemoryStats getStats();

private:
    vma::Allocator allocator;
    vk::PhysicalDeviceMemoryProperties properties;
};

}    // namespace pivot::graphics

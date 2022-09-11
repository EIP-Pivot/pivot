#pragma once

#include <span>
#include <string>
#include <vector>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/containers/IndexedStorage.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"
#include "pivot/graphics/vk_debug.hxx"
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

    /// The path where the json mem dump file will be written to.
    constexpr static auto memory_dump_file_name = "./vma_mem_dump.json";

public:
    /// Constructor
    VulkanAllocator();
    VulkanAllocator(const VulkanAllocator &) = delete;
    /// Destructor
    ~VulkanAllocator();

    /// Create the vma::Allocator object
    void init(const vma::AllocatorCreateInfo &info);
    /// Destory the vma::Allocator object. Does not free any allocated memory
    void destroy() { allocator.destroy(); }

    template <typename T>
    /// Create a buffer.
    AllocatedBuffer<T> createBuffer(std::size_t size, vk::BufferUsageFlags usage,
                                    vma::MemoryUsage memoryUsage = vma::MemoryUsage::eAuto,
                                    vma::AllocationCreateFlags flags = {}, const std::string &debug_name = "")
    {
        pivot_assert(size != 0, "Can't create empty buffer !");
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
        if (!debug_name.empty()) {
            vk_debug::setObjectName(device, buffer.buffer, debug_name);
            vk_debug::setObjectName(device, buffer.info.deviceMemory, debug_name + " Memory");
            allocator.setAllocationName(buffer.memory, debug_name.c_str());
        }
        return buffer;
    }

    template <Hashable K, typename T>
    IndexableBuffer<K, T> createIndexableBuffer(const IndexedStorage<K, T> source, vk::BufferUsageFlags usage,
                                                vma::MemoryUsage memoryUsage = vma::MemoryUsage::eAuto,
                                                vma::AllocationCreateFlags flags = {},
                                                const std::string &debug_name = "")
    {
        IndexableBuffer<K, T> storage = createBuffer<T>(source.size(), usage, memoryUsage, flags, debug_name);
        copyBuffer(storage, source.getStorage());
        storage.index = source.getIndexes();
        return storage;
    }

    template <typename T>
    /// Create a CPU mappable buffer
    AllocatedBuffer<T> createMappedBuffer(std::size_t bufferSize, const std::string &debug_name = "",
                                          vk::BufferUsageFlags usage = {})
    {
        return createBuffer<T>(bufferSize, vk::BufferUsageFlagBits::eStorageBuffer | usage, vma::MemoryUsage::eAuto,
                               vma::AllocationCreateFlagBits::eMapped |
                                   vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
                               debug_name);
    }

    /// @brief Create an image.
    ///
    /// The layout is undefined
    AllocatedImage createImage(const vk::ImageCreateInfo &info, const vma::AllocationCreateInfo &allocInfo);

    template <typename T>
    /// Map buffer memory to a pointer
    T *mapMemory(AllocatedBuffer<T> &buffer)
    {
        pivot_assert(buffer, "Buffer not created !");
        return static_cast<T *>(allocator.mapMemory(buffer.memory));
    }

    template <typename T>
    /// Map buffer memory as a read-only pointer
    const T *mapMemory(AllocatedBuffer<T> &buffer) const
    {
        pivot_assert(buffer, "Buffer not created !");
        return static_cast<const T *const>(allocator.mapMemory(buffer.memory));
    }

    template <typename T>
    /// @brief Unmap buffer memory.
    ///
    /// It must be called the same number of times mapMemory() is called.
    /// It is safe to call even if the buffer has been created with vma::AllocationCreateFlagBits::eMapped.
    void unmapMemory(AllocatedBuffer<T> &buffer)
    {
        pivot_assert(buffer, "Buffer not created !");
        allocator.unmapMemory(buffer.memory);
    }

    template <typename T>
    /// Copy the data into a buffer
    requires std::is_standard_layout_v<T>
    void copyBuffer(AllocatedBuffer<T> &buffer, const T *data, std::size_t data_size, std::size_t offset = 0)
    {
        if (data_size == 0) return;
        pivot_assert(buffer, "Buffer not created !");
        pivot_assert(buffer.getBytesSize() >= data_size + offset, "Buffer not big enough");
        pivot_assert(data, "Source data is NULL");
        pivot_assert((data_size + offset) % sizeof(T) == 0, "Source data is incorrectly aligned");

        auto *mapped = mapMemory<T>(buffer);
        std::memcpy(mapped + offset, data, data_size);
        unmapMemory(buffer);
    }

    template <BufferValid T>
    /// Copy the vector into the buffer
    void copyBuffer(AllocatedBuffer<T> &buffer, const std::span<T> &data, std::size_t offset = 0)
    {
        return copyBuffer(buffer, data.data(), data.size_bytes(), offset);
    }

    template <typename T>
    /// Destroy an allocated buffer
    void destroyBuffer(AllocatedBuffer<T> &buffer)
    {
        if (buffer) allocator.destroyBuffer(buffer.buffer, buffer.memory);
    }

    /// Destroy an image. Does not destroy its image view
    void destroyImage(AllocatedImage &image) { allocator.destroyImage(image.image, image.memory); }

    template <BufferValid T>
    /// Set allocation name
    void setAllocationName(AllocatedBuffer<T> &buffer, const std::string &name) const
    {
        allocator.setAllocationName(buffer.memory, name.c_str());
    }
    /// @copydoc setAllocationName
    void setAllocationName(AllocatedImage &image, const std::string &name) const
    {
        allocator.setAllocationName(image.memory, name.c_str());
    }

    /// Print memory status to the logger
    void dumpStats();
    /// Return the current status of the memory
    GPUMemoryStats getStats();

private:
    vk::Device device;
    vma::Allocator allocator;
    vk::PhysicalDeviceMemoryProperties properties;
};

}    // namespace pivot::graphics

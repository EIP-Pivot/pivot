#pragma once

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// @class AllocatedBuffer
///
/// @brief Utility class to keep track of a Vulkan buffer and its allocated memory
class AllocatedBuffer
{
public:
    /// get the size of the buffer
    auto getSize() const noexcept { return size; }
    /// get the allocated size (can be different from the size)
    auto getAllocatedSize() const noexcept { return info.size; }

    template <typename T>
    /// If the buffer was created with the flag vma::AllocationCreateFlagBits::eMapped, return the mapped pointer
    T *getMappedPointer() const noexcept
    {
        assert(flags & vma::AllocationCreateFlagBits::eMapped);
        return static_cast<T *>(info.pMappedData);
    }

    /// Test if the Vulkan buffer is created
    operator bool() const noexcept { return buffer && memory; }

public:
    //// @cond
    vk::Buffer buffer = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
    vk::DeviceSize size = 0;
    vma::AllocationInfo info;
    vma::AllocationCreateFlags flags;
    //// @endcond
};

}    // namespace pivot::graphics

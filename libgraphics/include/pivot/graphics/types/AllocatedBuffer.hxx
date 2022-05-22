#pragma once

#include <span>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

template <typename T>
/// Only accept hashable type
concept BufferValid =
    std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T> && requires
{
    sizeof(T) % 4 == 0;
};

template <BufferValid T>
/// @class AllocatedBuffer
///
/// @brief Utility class to keep track of a Vulkan buffer and its allocated memory
class AllocatedBuffer
{
public:
    /// get the number of valid item in the buffer
    auto getSize() const noexcept { return size; }
    /// get the allocated size (can be different from the size)
    auto getAllocatedSize() const noexcept { return info.size; }
    /// get the byte size
    auto getBytesSize() const noexcept { return getSize() * sizeof(T); }

    /// return the info struct used when creating a descriptor set
    /// @see DescriptorBuilder::bindBuffer
    vk::DescriptorBufferInfo getBufferInfo(vk::DeviceSize offset = 0) const noexcept
    {
        return {
            .buffer = buffer,
            .offset = offset,
            .range = getBytesSize(),
        };
    }

    /// @brief return the mapped pointer
    ///
    /// If the buffer was created with the flag vma::AllocationCreateFlagBits::eMapped, return the mapped pointer
    T *getMappedPointer() const noexcept
    {
        assert(flags & vma::AllocationCreateFlagBits::eMapped);
        return static_cast<T *>(info.pMappedData);
    }

    /// @copybrief getMappedPointer
    ///
    /// If the buffer was created with the flag vma::AllocationCreateFlagBits::eMapped, return the mapped pointer,
    /// wrapped into a span
    std::span<T> getMappedSpan() const noexcept { return std::span(getMappedPointer(), getBytesSize()); }

    /// Test if the Vulkan buffer is created
    operator bool() const noexcept { return buffer && memory && size > 0; }

public:
    //// @cond
    vk::Buffer buffer = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
    std::size_t size = 0;
    vma::AllocationInfo info = {};
    vma::AllocationCreateFlags flags = {};
    //// @endcond
};

}    // namespace pivot::graphics

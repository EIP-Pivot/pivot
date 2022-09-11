#pragma once

#include <span>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/pivot.hxx"
#include "pivot/utility/concept.hxx"

namespace pivot::graphics
{

template <typename T>
/// Check if the type is safe to use in a GLSL shader
concept GLSL_safe =
    std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T> && requires
{
    sizeof(T) % 4 == 0;
};

template <typename T>
/// Check if the type is usable in a GLSL shader and allow the void type
concept BufferValid = GLSL_safe<T> || std::is_same_v<T, void>;

template <BufferValid T>
/// @class AllocatedBuffer
///
/// @brief Utility class to keep track of a Vulkan buffer and its allocated memory
///
/// A correct type T is prefered, but giving void is allowed.
/// With `void` as template parameter, you will disable all type checking and memory safety
class AllocatedBuffer
{
public:
    /// Return true if the buffer can be accessed with getMappedSpan()
    constexpr bool isMappable() const noexcept { return bool(flags & vma::AllocationCreateFlagBits::eMapped); }

    /// get the number of valid item in the buffer
    constexpr auto getSize() const noexcept { return size; }
    /// get the allocated size (can be different from the size)
    constexpr auto getAllocatedSize() const noexcept { return info.size; }

    /// @brief get the byte size
    ///
    /// When `T` is not `void`, the size of buffer will be multiplied by `sizeof(T)`
    constexpr auto getBytesSize() const noexcept
    {
        if constexpr (std::is_same_v<T, void>) {
            return getSize();
        } else {
            return getSize() * sizeof(T);
        }
    }

    /// return the info struct used when creating a descriptor set
    /// @see DescriptorBuilder::bindBuffer
    constexpr vk::DescriptorBufferInfo getBufferInfo(vk::DeviceSize offset = 0) const noexcept
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
    constexpr T *getMappedPointer() const noexcept
    {
        pivot_assert(isMappable(), "Buffer was not created with the \"vma::AllocationCreateFlagBits::eMapped bit\"");
        return static_cast<T *>(info.pMappedData);
    }

    /// @copybrief getMappedPointer
    ///
    /// If the buffer was created with the flag vma::AllocationCreateFlagBits::eMapped, return the mapped pointer,
    /// wrapped into a span
    constexpr std::span<T> getMappedSpan() const noexcept { return std::span(getMappedPointer(), getBytesSize()); }

    /// Test if the Vulkan buffer is created
    constexpr operator bool() const noexcept { return buffer && memory && size > 0; }

    template <BufferValid O>
    /// Allow to convert AllocatedBuffer<void> to AllocatedBuffer<T>
    requires std::is_same_v<T, void>
    constexpr operator AllocatedBuffer<O>() const noexcept { return *this; }

    template <BufferValid O>
    /// Allow to convert AllocatedBuffer<void> to AllocatedBuffer<T>
    requires std::is_same_v<T, void> AllocatedBuffer<T>
    constexpr operator=(const AllocatedBuffer<O> &other) const noexcept
    {
        *this = other;
        return *this;
    }

public:
    //// @cond
    vk::Buffer buffer = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
    std::size_t size = 0;
    vma::AllocationInfo info = {};
    vma::AllocationCreateFlags flags = {};
    //// @endcond
};

template <Hashable T, BufferValid V>
/// @class IndexableBuffer
/// Hold a buffer, but the index of each element can be indexed using a key
class IndexableBuffer : public AllocatedBuffer<V>
{
public:
    std::uint32_t getIndex(const T &name) const { return index.at(name); }

private:
    std::unordered_map<T, std::uint32_t> index;
};

}    // namespace pivot::graphics

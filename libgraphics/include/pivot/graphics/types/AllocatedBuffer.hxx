#pragma once

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/abstract/AImmediateCommand.hxx"

namespace pivot::graphics
{
class VulkanBase;
class AllocatedImage;

/// @class AllocatedBuffer
///
/// @brief Utility class to keep track of a Vulkan buffer and its allocated memory
class AllocatedBuffer
{
public:
    /// Constructor
    AllocatedBuffer();
    /// Destructor
    ~AllocatedBuffer();

    /// Clone the buffer into a new one
    AllocatedBuffer cloneBuffer(VulkanBase &i, const vk::BufferUsageFlags usage, const vma::MemoryUsage memoryUsage);

    /// Copy buffer to image
    void copyToImage(abstract::AImmediateCommand &, AllocatedImage &dstImage) const;
    /// Test if the Vulkan buffer is created
    operator bool() const noexcept { return buffer && memory; }

public:
    //// @cond
    vk::Buffer buffer = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
    std::uint32_t size = 0;

    //// @endcond
};

}    // namespace pivot::graphics

#pragma once

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/VulkanBase.hxx"

namespace pivot::graphics
{

/// @class AllocatedBuffer
///
/// @brief Utility class to keep track of a Vulkan buffer and its allocated memory
class AllocatedBuffer
{
public:
    AllocatedBuffer();
    ~AllocatedBuffer();

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
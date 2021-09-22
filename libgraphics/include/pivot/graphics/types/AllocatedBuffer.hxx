#pragma once

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

/// @struct AllocatedBuffer
///
/// @brief Utility structure to keep track of a Vulkan buffer and its allocated memory
struct AllocatedBuffer {
    //// @cond
    vk::Buffer buffer = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
    //// @endcond
};

/// @struct AllocatedImage
///
/// @brief Utility structure to keep track of a Vulkan image,  its allocated memory et ImageView
struct AllocatedImage {
    //// @cond
    vk::Image image = VK_NULL_HANDLE;
    vk::ImageView imageView = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
    /// @endcond
};

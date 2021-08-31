#pragma once

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

struct AllocatedBuffer {
    vk::Buffer buffer = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
};

struct AllocatedImage {
    vk::Image image = VK_NULL_HANDLE;
    vk::ImageView imageView = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
};

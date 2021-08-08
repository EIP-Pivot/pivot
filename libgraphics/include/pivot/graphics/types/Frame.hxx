#pragma once

#include "pivot/graphics/types/AllocatedBuffer.hxx"

#include <vulkan/vulkan_core.h>

struct Frame {
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFences;
    AllocatedBuffer indirectBuffer{};
    struct {
        AllocatedBuffer uniformBuffers{};
        AllocatedBuffer materialBuffer{};
        VkDescriptorSet objectDescriptor = VK_NULL_HANDLE;
    } data = {};
};

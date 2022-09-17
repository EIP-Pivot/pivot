#pragma once

#include "pivot/graphics/AssetStorage/AssetStorage.hxx"
#include "pivot/graphics/DescriptorAllocator/DescriptorBuilder.hxx"
#include "pivot/graphics/DrawCallResolver.hxx"
#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

namespace pivot::graphics
{

/// @class Frame
/// @brief Store all of the per frame ressources
class Frame
{
public:
    Frame() = default;
    ~Frame() = default;

    /// Initialize the frame's ressources
    void initFrame(VulkanBase &base, DescriptorBuilder build, const AssetStorage &stor, vk::CommandPool &pool);
    /// Destroy the frame's ressources
    void destroy(VulkanBase &base, vk::CommandPool &pool);

public:
    /// Hold the DrawCallResolver for this frame
    DrawCallResolver drawResolver;
    /// Main command buffer
    vk::CommandBuffer cmdBuffer;
    /// Indicate if the image can be recover from the swapchain
    vk::Semaphore imageAvailableSemaphore;
    /// Indicate if the image has completed render
    vk::Semaphore renderFinishedSemaphore;
    /// Indicate if the image has been displayed and if it's ressources can be reused
    vk::Fence inFlightFences;
};

}    // namespace pivot::graphics

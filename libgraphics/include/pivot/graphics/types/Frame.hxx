#pragma once

#include "pivot/graphics/AssetStorage.hxx"
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

    void initFrame(VulkanBase &base, AssetStorage &stor);
    void destroy(VulkanBase &base);

public:
    DrawCallResolver drawResolver;
    /// Indicate if the image can be recover from the swapchain
    vk::Semaphore imageAvailableSemaphore;
    /// Indicate if the image has completed render
    vk::Semaphore renderFinishedSemaphore;
    /// Indicate if the image has been displayed and if it's ressources can be reused
    vk::Fence inFlightFences;
};

}    // namespace pivot::graphics

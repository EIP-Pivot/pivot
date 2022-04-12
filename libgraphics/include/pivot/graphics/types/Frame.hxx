#pragma once

#include "pivot/graphics/types/AllocatedBuffer.hxx"

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{
/// @struct Frame
/// @brief Store all of the per frame ressources
struct Frame {
    /// Indicate if the image can be recover from the swapchain
    vk::Semaphore imageAvailableSemaphore;
    /// Indicate if the image has completed render
    vk::Semaphore renderFinishedSemaphore;
    /// Indicate if the image has been displayed and if it's ressources can be reused
    vk::Fence inFlightFences;
};

}    // namespace pivot::graphics

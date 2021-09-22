#pragma once

#include "pivot/graphics/types/AllocatedBuffer.hxx"

#include <vulkan/vulkan.hpp>

/// @struct Frame
/// @brief Store all of the per frame ressources
struct Frame {
    /// Indicate if the image can be recover from the swapchain
    vk::Semaphore imageAvailableSemaphore;
    /// Indicate if the image has completed render
    vk::Semaphore renderFinishedSemaphore;
    /// Indicate if the image has been displayed and if it's ressources can be reused
    vk::Fence inFlightFences;
    /// The buffer for the indirect drawing
    AllocatedBuffer indirectBuffer{};

    /// This structure hold the actual data of the frame. This struct mostly exist to separate the syncronization
    /// objects from the rest.
    struct {
        /// The buffer for the information of the 3D object
        AllocatedBuffer uniformBuffer{};
        /// The descriptor set for to access to the uniformBuffer in the shader
        vk::DescriptorSet objectDescriptor = VK_NULL_HANDLE;
        /// The buffer for the materials of the 3D scene
        AllocatedBuffer materialBuffer{};

    } data = {};
};

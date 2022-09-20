#pragma once

#include "pivot/graphics/AssetStorage/AssetStorage.hxx"
#include "pivot/graphics/DescriptorAllocator/DescriptorBuilder.hxx"
#include "pivot/graphics/DrawCallResolver.hxx"
#include "pivot/graphics/LightDataResolver.hxx"
#include "pivot/graphics/ResolverDispatcher.hxx"
#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

namespace pivot::graphics
{

/// @class Frame
/// @brief Store all of the per frame ressources
class Frame
{
public:
    /// Initialize the frame's ressources
    void initFrame(VulkanBase &base, DescriptorBuilder build, const AssetStorage &stor, vk::CommandPool &pool);
    /// Destroy the frame's ressources
    void destroy(VulkanBase &base, vk::CommandPool &pool);

    template <typename T>
    /// Add a resolver to the frame
    FORCEINLINE void addResolver(unsigned setID)
    {
        dispatcher.addResolver<T>(setID);
    }

public:
    /// Hold the resolvers for this frame
    ResolverDispatcher dispatcher;
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

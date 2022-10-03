#include "pivot/graphics/types/Frame.hxx"
#include "pivot/pivot.hxx"

namespace pivot::graphics
{

void Frame::initFrame(VulkanBase &base, DescriptorBuilder build, const AssetStorage &stor, vk::CommandPool &pool)
{
    DEBUG_FUNCTION();
    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled,
    };
    imageAvailableSemaphore = base.device.createSemaphore(semaphoreInfo);
    renderFinishedSemaphore = base.device.createSemaphore(semaphoreInfo);
    inFlightFences = base.device.createFence(fenceInfo);

    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };
    cmdBuffer = base.device.allocateCommandBuffers(allocInfo).front();

    dispatcher.initialize(base, stor, build);
}

void Frame::destroy(VulkanBase &base, vk::CommandPool &pool)
{
    DEBUG_FUNCTION();
    dispatcher.destroy(base);
    base.device.freeCommandBuffers(pool, cmdBuffer);
    base.device.destroyFence(inFlightFences);
    base.device.destroySemaphore(renderFinishedSemaphore);
    base.device.destroySemaphore(imageAvailableSemaphore);
}

}    // namespace pivot::graphics

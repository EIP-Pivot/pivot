#include "pivot/graphics/types/Frame.hxx"

#include "pivot/graphics/DebugMacros.hxx"

namespace pivot::graphics
{

void Frame::initFrame(VulkanBase &base, AssetStorage &stor)
{
    DEBUG_FUNCTION
    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled,
    };
    imageAvailableSemaphore = base.device.createSemaphore(semaphoreInfo);
    renderFinishedSemaphore = base.device.createSemaphore(semaphoreInfo);
    inFlightFences = base.device.createFence(fenceInfo);

    drawResolver.init(base, stor);
}

void Frame::destroy(VulkanBase &base)
{
    drawResolver.destroy();
    base.device.destroy(inFlightFences);
    base.device.destroy(renderFinishedSemaphore);
    base.device.destroy(imageAvailableSemaphore);
}

}    // namespace pivot::graphics

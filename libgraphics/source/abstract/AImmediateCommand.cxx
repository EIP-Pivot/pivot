#include "pivot/graphics/abstract/AImmediateCommand.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics::abstract
{
AImmediateCommand::AImmediateCommand() {}

AImmediateCommand::~AImmediateCommand() { destroy(); }

void AImmediateCommand::init(vk::Device &device, const uint32_t queueFamilyIndex)
{
    device_ref = device;

    createImmediateContext(queueFamilyIndex);
    immediateQueue = device_ref->get().getQueue(queueFamilyIndex, 0);
}

void AImmediateCommand::destroy()
{
    if (device_ref) {
        device_ref->get().destroyCommandPool(immediateCommandPool);
        device_ref->get().destroyFence(immediateFence);
    }
    device_ref = std::nullopt;
}

void AImmediateCommand::immediateCommand(std::function<void(vk::CommandBuffer &)> function)
{
    assert(immediateCommandPool && immediateFence && immediateQueue);
    vk::CommandBufferAllocateInfo cmdAllocInfo{
        .commandPool = immediateCommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };
    vk::CommandBuffer cmd = device_ref->get().allocateCommandBuffers(cmdAllocInfo)[0];

    vk::CommandBufferBeginInfo cmdBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };

    cmd.begin(cmdBeginInfo);
    function(cmd);
    cmd.end();

    vk::SubmitInfo submit{
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };

    immediateQueue.submit(submit, immediateFence);
    vk_utils::vk_try(device_ref->get().waitForFences(immediateFence, VK_TRUE, UINT64_MAX));
    device_ref->get().resetFences(immediateFence);
    device_ref->get().resetCommandPool(immediateCommandPool);
}
void AImmediateCommand::createImmediateContext(const uint32_t queueFamilyIndex)
{
    vk::FenceCreateInfo fenceInfo{};
    immediateFence = device_ref->get().createFence(fenceInfo);

    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueFamilyIndex,
    };
    immediateCommandPool = device_ref->get().createCommandPool(poolInfo);
}

}    // namespace pivot::graphics::abstract
#include "pivot/graphics/VulkanImmediateCommand.hxx"

#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics
{
VulkanImmediateCommand::VulkanImmediateCommand() {}

VulkanImmediateCommand::~VulkanImmediateCommand() { destroy(); }

void VulkanImmediateCommand::init(vk::Device &device, const vk::PhysicalDevice &gpu,
                                  const QueueFamilyIndices &queueFamilyIndex)
{
    device_ref = device;
    physical_device_ref = gpu;

    createImmediateContext(queueFamilyIndex);
    vk_debug::setObjectName(device_ref->get(), queues.at(vk::QueueFlagBits::eGraphics).queue,
                            "Immediate Graphics Queue");
    vk_debug::setObjectName(device_ref->get(), queues.at(vk::QueueFlagBits::eGraphics).pool,
                            "Immediate Graphics Command Pool");

    vk_debug::setObjectName(device_ref->get(), queues.at(vk::QueueFlagBits::eCompute).queue, "Immediate Compute Queue");
    vk_debug::setObjectName(device_ref->get(), queues.at(vk::QueueFlagBits::eCompute).pool,
                            "Immediate Compute Command Pool");

    vk_debug::setObjectName(device_ref->get(), queues.at(vk::QueueFlagBits::eTransfer).queue,
                            "Immediate Transfer Queue");
    vk_debug::setObjectName(device_ref->get(), queues.at(vk::QueueFlagBits::eTransfer).pool,
                            "Immediate Transfer Command Pool");
}

void VulkanImmediateCommand::destroy()
{
    if (device_ref) {
        for (const auto &[_, queue_data]: queues) device_ref->get().destroyCommandPool(queue_data.pool);
        device_ref->get().destroyFence(immediateFence);
    }
    device_ref = std::nullopt;
}

void VulkanImmediateCommand::immediateCommand(std::function<void(vk::CommandBuffer &)> function,
                                              vk::QueueFlagBits requiredQueue, const std::source_location &location)
{
    pivot_assert(queues.size() == 3 && queues.contains(requiredQueue) && queues.at(requiredQueue).pool &&
                     queues.at(requiredQueue).queue,
                 "Immediate context is not initialised");

    const auto &[queue, pool] = queues.at(requiredQueue);

    vk::CommandBufferAllocateInfo cmdAllocInfo{
        .commandPool = pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };
    vk::CommandBuffer cmd = device_ref->get().allocateCommandBuffers(cmdAllocInfo).front();
    vk_debug::setObjectName(device_ref->get(), cmd, location.function_name());

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
#ifndef NDEBUG
    vk::DebugUtilsLabelEXT label{
        .pLabelName = location.function_name(),
    };
    queues.at(requiredQueue).queue.beginDebugUtilsLabelEXT(label);
#endif

    queues.at(requiredQueue).queue.submit(submit, immediateFence);

#ifndef NDEBUG
    queues.at(requiredQueue).queue.endDebugUtilsLabelEXT();
#endif
    vk_utils::vk_try(device_ref->get().waitForFences(immediateFence, VK_TRUE, UINT64_MAX));
    device_ref->get().resetFences(immediateFence);
    device_ref->get().resetCommandPool(pool);
}

void VulkanImmediateCommand::copyImageToImage(const AllocatedImage &srcBuffer, AllocatedImage &dstImage)
{
    pivot_assert(srcBuffer.size == dstImage.size, "The size of the image are not the same");
    immediateCommand(
        [&](vk::CommandBuffer &cmd) {
            vk::ImageCopy region{
                .srcOffset = {0, 0, 0},
                .dstOffset = {0, 0, 0},
                .extent = srcBuffer.size,
            };
            cmd.copyImage(srcBuffer.image, srcBuffer.imageLayout, dstImage.image, dstImage.imageLayout, region);
        },
        vk::QueueFlagBits::eTransfer);
}

void VulkanImmediateCommand::copyBufferToImage(const AllocatedBuffer<std::byte> &srcBuffer, AllocatedImage &dstImage)
{
    immediateCommand(
        [&](vk::CommandBuffer &cmd) {
            vk::BufferImageCopy region{
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource =
                    {
                        .aspectMask = vk::ImageAspectFlagBits::eColor,
                        .mipLevel = 0,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                .imageOffset = {0, 0, 0},
                .imageExtent = dstImage.size,
            };
            cmd.copyBufferToImage(srcBuffer.buffer, dstImage.image, vk::ImageLayout::eTransferDstOptimal, region);
        },
        vk::QueueFlagBits::eTransfer);
}

void VulkanImmediateCommand::generateMipmaps(AllocatedImage &image, uint32_t mipLevel)
{
    vk::FormatProperties formatProperties = physical_device_ref->get().getFormatProperties(image.format);
    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        logger.warn("VulkanImmediateCommand")
            << "Failed to generate mipmaps : texture image format does not support linear tilting!";
        return;
    }

    immediateCommand(
        [&](vk::CommandBuffer &cmd) {
            vk::ImageMemoryBarrier barrier{
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image.image,
                .subresourceRange =
                    {
                        .aspectMask = vk::ImageAspectFlagBits::eColor,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
            };
            std::int32_t mipWidth = image.size.width;
            std::int32_t mipHeight = image.size.height;

            for (std::uint32_t i = 1; i < mipLevel; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

                cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {},
                                    {}, barrier);

                vk::ImageBlit blit{};
                blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
                blit.srcOffsets[1] = vk::Offset3D{
                    .x = mipWidth,
                    .y = mipHeight,
                    .z = 1,
                };
                blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
                blit.dstOffsets[1] = vk::Offset3D{
                    .x = std::max(1, mipWidth / 2),
                    .y = std::max(1, mipHeight / 2),
                    .z = 1,
                };
                blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                cmd.blitImage(image.image, vk::ImageLayout::eTransferSrcOptimal, image.image,
                              vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

                barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                    {}, {}, {}, barrier);

                if (mipWidth > 1) mipWidth /= 2;
                if (mipHeight > 1) mipHeight /= 2;
            }

            barrier.subresourceRange.baseMipLevel = mipLevel - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
                                {}, {}, barrier);
        },
        vk::QueueFlagBits::eGraphics);
    image.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    image.mipLevels = mipLevel;
}

void VulkanImmediateCommand::transitionLayout(AllocatedImage &image, vk::ImageLayout layout)
{
    if (image.imageLayout == layout) {
        logger.warn("Transition layout") << "Transfert layout is unecessary, already " << vk::to_string(layout);
        return;
    }
    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;
    vk::ImageMemoryBarrier barrier{
        .srcAccessMask = vk::AccessFlagBits::eNoneKHR,
        .dstAccessMask = vk::AccessFlagBits::eNoneKHR,
        .oldLayout = image.imageLayout,
        .newLayout = layout,
        .image = image.image,
        .subresourceRange =
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = image.mipLevels,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },

    };

    if (layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (image.format == vk::Format::eD32SfloatS8Uint || image.format == vk::Format::eD24UnormS8Uint) {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    if (image.imageLayout == vk::ImageLayout::eUndefined && layout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (image.imageLayout == vk::ImageLayout::eTransferDstOptimal &&
               layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (image.imageLayout == vk::ImageLayout::eUndefined &&
               layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = vk ::AccessFlagBits::eNoneKHR;
        barrier.dstAccessMask =
            vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    } else {
        throw VulkanImmediateCommandError("Unsupported layout transition! From " + vk::to_string(image.imageLayout) +
                                          " to " + vk::to_string(layout));
    }

    immediateCommand(
        [&](vk::CommandBuffer &cmd) { cmd.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier); },
        vk::QueueFlagBits::eGraphics);
    image.imageLayout = layout;
}

void VulkanImmediateCommand::createImmediateContext(const QueueFamilyIndices &queueFamilyIndex)
{
    vk::FenceCreateInfo fenceInfo{};
    immediateFence = device_ref->get().createFence(fenceInfo);

    queues = {
        {
            vk::QueueFlagBits::eGraphics,
            {
                .queue = device_ref->get().getQueue(queueFamilyIndex.graphicsFamily.value(), 0),
                .pool = device_ref->get().createCommandPool({
                    .queueFamilyIndex = queueFamilyIndex.graphicsFamily.value(),
                }),
            },
        },
        {
            vk::QueueFlagBits::eCompute,
            {
                .queue = device_ref->get().getQueue(queueFamilyIndex.computeFamily.value(), 0),
                .pool = device_ref->get().createCommandPool({
                    .queueFamilyIndex = queueFamilyIndex.computeFamily.value(),
                }),
            },
        },
        {
            vk::QueueFlagBits::eTransfer,
            {
                .queue = device_ref->get().getQueue(queueFamilyIndex.transferFamily.value(), 0),
                .pool = device_ref->get().createCommandPool({
                    .queueFamilyIndex = queueFamilyIndex.transferFamily.value(),
                }),
            },
        },
    };
}

}    // namespace pivot::graphics

#include "pivot/graphics/abstract/AImmediateCommand.hxx"

#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics::abstract
{
AImmediateCommand::AImmediateCommand() {}

AImmediateCommand::~AImmediateCommand() { destroy(); }

void AImmediateCommand::init(vk::Device &device, const vk::PhysicalDevice &gpu, const uint32_t queueFamilyIndex)
{
    device_ref = device;
    physical_device_ref = gpu;

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

void AImmediateCommand::immediateCommand(std::function<void(vk::CommandBuffer &)> function,
                                         const std::source_location &location)
{
    assert(immediateCommandPool && immediateFence && immediateQueue);
    vk::CommandBufferAllocateInfo cmdAllocInfo{
        .commandPool = immediateCommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };
    vk::CommandBuffer cmd = device_ref->get().allocateCommandBuffers(cmdAllocInfo)[0];
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

    immediateQueue.submit(submit, immediateFence);
    vk_utils::vk_try(device_ref->get().waitForFences(immediateFence, VK_TRUE, UINT64_MAX));
    device_ref->get().resetFences(immediateFence);
    device_ref->get().resetCommandPool(immediateCommandPool);
}

void AImmediateCommand::copyBufferToImage(const AllocatedBuffer<std::byte> &srcBuffer, AllocatedImage &dstImage)
{
    immediateCommand([&](vk::CommandBuffer &cmd) {
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
    });
}

void AImmediateCommand::generateMipmaps(AllocatedImage &image, uint32_t mipLevel)
{
    vk::FormatProperties formatProperties = physical_device_ref->get().getFormatProperties(image.format);
    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        throw std::runtime_error("texture image format does not support linear tilting!");
    }

    immediateCommand([&](vk::CommandBuffer &cmd) {
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
        int32_t mipWidth = image.size.width;
        int32_t mipHeight = image.size.height;

        for (uint32_t i = 1; i < mipLevel; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {},
                                barrier);

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

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
                                {}, {}, barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevel - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {},
                            {}, barrier);
    });
    image.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    image.mipLevels = mipLevel;
}

void AImmediateCommand::transitionLayout(AllocatedImage &image, vk::ImageLayout layout)
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
        throw ImmediateCommandError("Unsupported layout transition! From " + vk::to_string(image.imageLayout) + " to " +
                                    vk::to_string(layout));
    }

    immediateCommand(
        [&](vk::CommandBuffer &cmd) { cmd.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier); });
    image.imageLayout = layout;
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

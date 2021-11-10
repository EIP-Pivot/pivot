#include "pivot/graphics/VulkanApplication.hxx"

#include <functional>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/vk_utils.hxx"

float VulkanApplication::getAspectRatio() const { return swapchain.getAspectRatio(); }
void VulkanApplication::copyBufferToBuffer(const vk::Buffer &srcBuffer, vk::Buffer &dstBuffer,
                                           const vk::DeviceSize &size)
{
    DEBUG_FUNCTION
    immediateCommand([=](vk::CommandBuffer &cmd) {
        vk::BufferCopy copyRegion{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };
        cmd.copyBuffer(srcBuffer, dstBuffer, copyRegion);
    });
}
void VulkanApplication::copyBufferToImage(const vk::Buffer &srcBuffer, vk::Image &dstImage, const vk::Extent3D &extent)
{
    DEBUG_FUNCTION
    immediateCommand([=](vk::CommandBuffer &cmd) {
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
            .imageExtent = extent,
        };
        cmd.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, region);
    });
}

void VulkanApplication::immediateCommand(std::function<void(vk::CommandBuffer &)> &&function)
{
    DEBUG_FUNCTION

    if (!uploadContext.commandPool || !uploadContext.uploadFence)
        throw VulkanException("The immediate context is not initialised!");

    vk::CommandBufferAllocateInfo cmdAllocInfo{
        .commandPool = uploadContext.commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    vk::CommandBuffer cmd = device.allocateCommandBuffers(cmdAllocInfo)[0];

    vk::CommandBufferBeginInfo cmdBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };

    cmd.begin(cmdBeginInfo);
    function(cmd);
    cmd.end();

    vk::SubmitInfo submit;
    submit.setCommandBuffers(cmd);

    graphicsQueue.submit(submit, uploadContext.uploadFence);
    VK_TRY(device.waitForFences(uploadContext.uploadFence, VK_TRUE, UINT64_MAX));
    device.resetFences(uploadContext.uploadFence);
    device.resetCommandPool(uploadContext.commandPool);
}

void VulkanApplication::transitionImageLayout(vk::Image &image, vk::Format format, vk::ImageLayout oldLayout,
                                              vk::ImageLayout newLayout, uint32_t mipLevels)
{
    DEBUG_FUNCTION
    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;
    vk::ImageMemoryBarrier barrier{
        .srcAccessMask = vk::AccessFlagBits::eNoneKHR,
        .dstAccessMask = vk::AccessFlagBits::eNoneKHR,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .image = image,
        .subresourceRange =
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = mipLevels,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },

    };

    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

        if (vk_utils::hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (oldLayout == vk::ImageLayout::eUndefined &&
               newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = vk ::AccessFlagBits::eNoneKHR;
        barrier.dstAccessMask =
            vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    immediateCommand([&](vk::CommandBuffer &cmd) {
        cmd.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, barrier);
    });
}

AllocatedBuffer VulkanApplication::createBuffer(uint32_t allocSize, vk::BufferUsageFlags usage,
                                                vma::MemoryUsage memoryUsage)
{
    DEBUG_FUNCTION
    vk::BufferCreateInfo bufferInfo{
        .size = allocSize,
        .usage = usage,
    };
    vma::AllocationCreateInfo vmaallocInfo;
    vmaallocInfo.usage = memoryUsage;

    AllocatedBuffer newBuffer;
    std::tie(newBuffer.buffer, newBuffer.memory) = allocator.createBuffer(bufferInfo, vmaallocInfo);
    return newBuffer;
}

void VulkanApplication::generateMipmaps(vk::Image &image, vk::Format imageFormat, vk::Extent3D size, uint32_t mipLevel)
{
    DEBUG_FUNCTION
    vk::FormatProperties formatProperties = physical_device.getFormatProperties(imageFormat);
    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        throw std::runtime_error("texture image format does not support linear tilting!");
    }

    immediateCommand([&](vk::CommandBuffer &cmd) {
        vk::ImageMemoryBarrier barrier{
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange =
                {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        int32_t mipWidth = size.width;
        int32_t mipHeight = size.height;

        for (uint32_t i = 1; i < mipLevel; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                                vk::DependencyFlags{}, nullptr, nullptr, barrier);

            vk::ImageBlit blit{};
            blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
            blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
            blit.dstOffsets[1] = vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
            blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            cmd.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, 1,
                          &blit, vk::Filter::eLinear);

            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                vk::DependencyFlags{}, nullptr, nullptr, barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevel - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                            vk::DependencyFlags{}, nullptr, nullptr, barrier);
    });
}

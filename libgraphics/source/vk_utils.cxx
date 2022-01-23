#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>
#include <fstream>
#include <stdexcept>

#include "pivot/graphics/vk_init.hxx"

namespace pivot::graphics::vk_utils
{

AllocatedBuffer createBuffer(vma::Allocator &allocator, uint32_t allocSize, vk::BufferUsageFlags usage,
                             vma::MemoryUsage memoryUsage)
{
    if (allocSize == 0) {
        LOGGER_ERR << "Can't allocate empty buffer !";

        abort();
    }
    vk::BufferCreateInfo bufferInfo{
        .size = allocSize,
        .usage = usage,
    };
    vma::AllocationCreateInfo vmaallocInfo;
    vmaallocInfo.usage = memoryUsage;

    AllocatedBuffer newBuffer{
        .size = allocSize,
    };
    std::tie(newBuffer.buffer, newBuffer.memory) = allocator.createBuffer(bufferInfo, vmaallocInfo);
    return newBuffer;
}

void copyBufferToBuffer(abstract::AImmediateCommand &i, const vk::Buffer &srcBuffer, vk::Buffer &dstBuffer,
                        const vk::DeviceSize &size)
{
    i.immediateCommand([=](vk::CommandBuffer &cmd) {
        vk::BufferCopy copyRegion{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };
        cmd.copyBuffer(srcBuffer, dstBuffer, copyRegion);
    });
}
void copyBufferToImage(abstract::AImmediateCommand &i, const vk::Buffer &srcBuffer, vk::Image &dstImage,
                       const vk::Extent3D &extent)
{
    i.immediateCommand([=](vk::CommandBuffer &cmd) {
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

void transitionImageLayout(abstract::AImmediateCommand &i, vk::Image &image, vk::Format format,
                           vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels)
{
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

    i.immediateCommand([&](vk::CommandBuffer &cmd) {
        cmd.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, barrier);
    });
}

void generateMipmaps(VulkanBase &base, vk::Image &image, vk::Format imageFormat, vk::Extent3D size, uint32_t mipLevel)
{
    vk::FormatProperties formatProperties = base.physical_device.getFormatProperties(imageFormat);
    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        throw std::runtime_error("texture image format does not support linear tilting!");
    }

    base.immediateCommand([&](vk::CommandBuffer &cmd) {
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

std::vector<std::byte> readFile(const std::string &filename)
{
    size_t fileSize = 0;
    std::vector<std::byte> fileContent;
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) { throw std::runtime_error("failed to open file " + filename); }
    fileSize = file.tellg();
    fileContent.resize(fileSize);
    file.seekg(0);
    file.read((char *)fileContent.data(), fileSize);
    file.close();
    return fileContent;
}

vk::ShaderModule createShaderModule(const vk::Device &device, const std::vector<std::byte> &code)
{
    auto createInfo = vk_init::populateVkShaderModuleCreateInfo(code);
    return device.createShaderModule(createInfo);
}

vk::SampleCountFlagBits getMexUsableSampleCount(vk::PhysicalDevice &physical_device)
{
    vk::PhysicalDeviceProperties physicalDeviceProperties = physical_device.getProperties();

    vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                  physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
    if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
    if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
    if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
    if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
    if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }

    return vk::SampleCountFlagBits::e1;
}

vk::Format findSupportedFormat(vk::PhysicalDevice &gpu, const std::vector<vk::Format> &candidates,
                               vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    for (vk::Format format: candidates) {
        vk::FormatProperties props = gpu.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format");
}

bool hasStencilComponent(vk::Format format) noexcept
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

namespace tools
{
    const std::string to_string(vk::SampleCountFlagBits count) noexcept
    {
        switch (count) {
            case vk::SampleCountFlagBits::e1: return "No MSAA";
            case vk::SampleCountFlagBits::e2: return "2X MSAA";
            case vk::SampleCountFlagBits::e4: return "4X MSAA";
            case vk::SampleCountFlagBits::e8: return "8X MSAA";
            case vk::SampleCountFlagBits::e16: return "16X MSAA";
            case vk::SampleCountFlagBits::e32: return "32X MSAA";
            case vk::SampleCountFlagBits::e64: return "64X MSAA";
            default: return "Unknown";
        }
    }
    const std::string to_string(vk::CullModeFlagBits count) noexcept
    {
        switch (count) {
            case vk::CullModeFlagBits::eNone: return "No culling";
            case vk::CullModeFlagBits::eBack: return "Back culling";
            case vk::CullModeFlagBits::eFront: return "Front culling";
            case vk::CullModeFlagBits::eFrontAndBack: return "Both side culling";
            default: return "Unknown";
        }
    }

    std::string physicalDeviceTypeString(vk::PhysicalDeviceType type) noexcept
    {
        switch (type) {
#define STR(r) \
    case vk::PhysicalDeviceType::e##r: return #r
            STR(Other);
            STR(IntegratedGpu);
            STR(DiscreteGpu);
            STR(VirtualGpu);
            STR(Cpu);
#undef STR
            default: return "UNKNOWN_DEVICE_TYPE";
        }
    }
}    // namespace tools
}    // namespace pivot::graphics::vk_utils

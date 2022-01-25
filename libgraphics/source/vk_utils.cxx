#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>
#include <fstream>
#include <stdexcept>

#include "pivot/graphics/vk_init.hxx"

namespace pivot::graphics::vk_utils
{

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

vk::SampleCountFlagBits getMaxUsableSampleCount(vk::PhysicalDevice &physical_device)
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

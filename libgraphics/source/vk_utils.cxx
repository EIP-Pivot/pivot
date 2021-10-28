#include "pivot/graphics/vk_utils.hxx"

#include <fstream>
#include <stdexcept>

#include "pivot/graphics/vk_init.hxx"

namespace vk_utils
{
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
}    // namespace vk_utils

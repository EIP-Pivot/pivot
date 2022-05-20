#include "pivot/graphics/vk_utils.hxx"

#include <fstream>
#include <stdexcept>

#include "pivot/graphics/vk_init.hxx"

namespace pivot::graphics::vk_utils
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

vk::ShaderModule createShaderModule(const vk::Device &device, const std::span<std::byte> &code)
{
    auto createInfo = vk_init::populateVkShaderModuleCreateInfo(code);
    return device.createShaderModule(createInfo);
}

vk::SampleCountFlagBits getMaxUsableSampleCount(vk::PhysicalDevice &physical_device)
{
    vk::PhysicalDeviceProperties physicalDeviceProperties = physical_device.getProperties();

    vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                  physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & vk::SampleCountFlagBits::e64) return vk::SampleCountFlagBits::e64;
    if (counts & vk::SampleCountFlagBits::e32) return vk::SampleCountFlagBits::e32;
    if (counts & vk::SampleCountFlagBits::e16) return vk::SampleCountFlagBits::e16;
    if (counts & vk::SampleCountFlagBits::e8) return vk::SampleCountFlagBits::e8;
    if (counts & vk::SampleCountFlagBits::e4) return vk::SampleCountFlagBits::e4;
    if (counts & vk::SampleCountFlagBits::e2) return vk::SampleCountFlagBits::e2;

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

namespace tools
{
    std::string bytesToString(uint64_t bytes)
    {
        constexpr uint64_t GB = 1024 * 1024 * 1024;
        constexpr uint64_t MB = 1024 * 1024;
        constexpr uint64_t KB = 1024;

        std::stringstream buffer;
        buffer.precision(2);

        if (bytes > GB)
            buffer << uint64_t(float(bytes) / float(GB)) << " GB";
        else if (bytes > MB)
            buffer << uint64_t(float(bytes) / float(MB)) << " MB";
        else if (bytes > KB)
            buffer << uint64_t(float(bytes) / float(KB)) << " KB";
        else
            buffer << uint64_t(float(bytes) / float(MB)) << " bytes";
        return buffer.str();
    }

}    // namespace tools
}    // namespace pivot::graphics::vk_utils

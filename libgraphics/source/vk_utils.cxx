#include "pivot/graphics/vk_utils.hxx"

#include <fstream>

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

VkShaderModule createShaderModule(const VkDevice &device, const std::vector<std::byte> &code)
{
    auto createInfo = vk_init::populateVkShaderModuleCreateInfo(code);
    VkShaderModule shader = VK_NULL_HANDLE;
    VK_TRY(vkCreateShaderModule(device, &createInfo, nullptr, &shader));
    return shader;
}

VkFormat findSupportedFormat(VkPhysicalDevice &gpu, const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                             VkFormatFeatureFlags features)
{
    for (VkFormat format: candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(gpu, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format");
}

bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t findMemoryType(VkPhysicalDevice &physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties)) { return i; }
    }
    throw VulkanException("failed to find suitable memory type !");
}

std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance &instance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    if (devices.empty()) throw VulkanException("failed to find GPUs with Vulkan support");
    return devices;
}

namespace tools
{
    const std::string to_string(VkSampleCountFlagBits count)
    {
        switch (count) {
            case VK_SAMPLE_COUNT_1_BIT: return "No MSAA";
            case VK_SAMPLE_COUNT_2_BIT: return "2X MSAA";
            case VK_SAMPLE_COUNT_4_BIT: return "4X MSAA";
            case VK_SAMPLE_COUNT_8_BIT: return "8X MSAA";
            case VK_SAMPLE_COUNT_16_BIT: return "16X MSAA";
            case VK_SAMPLE_COUNT_32_BIT: return "32X MSAA";
            case VK_SAMPLE_COUNT_64_BIT: return "64X MSAA";
            default: return "Unknown";
        }
    }
    const std::string to_string(VkCullModeFlags count)
    {
        switch (count) {
            case VK_CULL_MODE_NONE: return "No culling";
            case VK_CULL_MODE_BACK_BIT: return "Back culling";
            case VK_CULL_MODE_FRONT_BIT: return "Front culling";
            case VK_CULL_MODE_FRONT_AND_BACK: return "Both side culling";
            default: return "Unknown";
        }
    }

    std::string physicalDeviceTypeString(VkPhysicalDeviceType type)
    {
        switch (type) {
#define STR(r) \
    case VK_PHYSICAL_DEVICE_TYPE_##r: return #r
            STR(OTHER);
            STR(INTEGRATED_GPU);
            STR(DISCRETE_GPU);
            STR(VIRTUAL_GPU);
            STR(CPU);
#undef STR
            default: return "UNKNOWN_DEVICE_TYPE";
        }
    }
}    // namespace tools
}    // namespace vk_utils

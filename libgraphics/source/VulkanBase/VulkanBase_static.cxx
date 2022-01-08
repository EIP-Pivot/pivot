#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/VulkanSwapchain.hxx"
#include "pivot/graphics/types/vk_types.hxx"

#include <Logger.hpp>
#include <ostream>
#include <set>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

constexpr static const char *to_string_message_type(const VkDebugUtilsMessageTypeFlagsEXT &s)
{
    if (s == 7) return "General | Validation | Performance";
    if (s == 6) return "Validation | Performance";
    if (s == 5) return "General | Performance";
    if (s == 4 /*VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT*/) return "Performance";
    if (s == 3) return "General | Validation";
    if (s == 2 /*VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT*/) return "Validation";
    if (s == 1 /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT*/) return "General";
    return "Unknown";
}

namespace pivot::graphics
{

uint32_t VulkanBase::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                   VkDebugUtilsMessageTypeFlagsEXT messageType,
                                   const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    std::stringstream &(Logger::*severity)(const std::string &) = nullptr;
    switch (messageSeverity) {
        case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            severity = &Logger::debug;
            break;
        case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            severity = &Logger::err;
            break;
        case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            severity = &Logger::warn;
            break;
        case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            severity = &Logger::info;
            break;
        default: severity = &Logger::err; break;
    }
    vk::to_string(vk::DebugUtilsMessageTypeFlagsEXT(messageType));
    (logger->*severity)(to_string_message_type(messageType)) << pCallbackData->pMessage;
    logger->endl();
    return VK_FALSE;
}

bool VulkanBase::checkValidationLayerSupport(const std::vector<const char *> &validationLayers)
{
    auto availableLayers = vk::enumerateInstanceLayerProperties();

    for (const char *layerName: validationLayers) {
        bool layerFound = false;

        for (const auto &layerProperties: availableLayers) {
            if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) return false;
    }
    return true;
}

bool VulkanBase::isDeviceSuitable(const vk::PhysicalDevice &gpu, const vk::SurfaceKHR &surface,
                                  const std::vector<const char *> &deviceExtensions)
{
    DEBUG_FUNCTION
    auto indices = QueueFamilyIndices::findQueueFamilies(gpu, surface);
    bool extensionsSupported = checkDeviceExtensionSupport(gpu, deviceExtensions);
    auto deviceProperties = gpu.getProperties();
    auto deviceFeatures = gpu.getFeatures();

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        auto swapChainSupport = VulkanSwapchain::SupportDetails::querySwapChainSupport(gpu, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    return indices.isComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy &&
           deviceProperties.limits.maxPushConstantsSize >= gpuObject::pushConstantsSize;
}

uint32_t VulkanBase::rateDeviceSuitability(const vk::PhysicalDevice &gpu)
{
    vk::PhysicalDeviceProperties deviceProperties = gpu.getProperties();
    vk::PhysicalDeviceFeatures deviceFeatures = gpu.getFeatures();
    int32_t score = 0;

    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) score += 1000;
    if (deviceFeatures.multiDrawIndirect == VK_TRUE) score += 100;

    score += deviceProperties.limits.maxPushConstantsSize;
    score += deviceProperties.limits.maxImageDimension2D;
    return score;
}

bool VulkanBase::checkDeviceExtensionSupport(const vk::PhysicalDevice &device,
                                             const std::vector<const char *> &deviceExtensions)
{
    DEBUG_FUNCTION
    auto availableExtensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto &extension: availableExtensions) { requiredExtensions.erase(extension.extensionName); }
    return requiredExtensions.empty();
}

}    // namespace pivot::graphics
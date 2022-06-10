#include "pivot/graphics/VulkanBase.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/PivotException.hxx"
#include "pivot/graphics/Window.hxx"
#include "pivot/graphics/pivot.hxx"
#include "pivot/graphics/types/vk_types.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <cstdint>
#include <map>
#include <set>

namespace pivot::graphics
{

void VulkanBase::createInstance(const std::vector<const char *> &instanceExtensions,
                                const std::vector<const char *> &validationLayers)
{
    DEBUG_FUNCTION
    auto debugInfo = vk_init::populateDebugUtilsMessengerCreateInfoEXT(&VulkanBase::debugCallback);
    auto extensions = Window::getRequiredExtensions();
    std::copy(instanceExtensions.begin(), instanceExtensions.end(), std::back_inserter(extensions));
    if (bEnableValidationLayers) { extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

    vk::ApplicationInfo applicationInfo{
        .pEngineName = "Pivot",
        .apiVersion = VK_API_VERSION_1_1,
    };
    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &applicationInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    if (bEnableValidationLayers) {
        createInfo.pNext = &debugInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    logger.info("Instance") << "Instance extensions: " << extensions;
    this->VulkanLoader::createInstance(createInfo);
    baseDeletionQueue.push([&] { instance.destroy(); });
}

void VulkanBase::createDebugMessenger()
{
    DEBUG_FUNCTION
    if (!bEnableValidationLayers) return;
    auto debugInfo = vk_init::populateDebugUtilsMessengerCreateInfoEXT(&VulkanBase::debugCallback);
    debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(debugInfo);

    logger.warn("Validation Layers") << "Validation Layers are activated !";
    baseDeletionQueue.push([&] { instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger); });
}

void VulkanBase::createSurface()
{
    DEBUG_FUNCTION
    surface = window.createSurface(instance);
    baseDeletionQueue.push([&] { instance.destroy(surface); });
}

void VulkanBase::selectPhysicalDevice(const std::vector<const char *> &deviceExtensions)
{
    DEBUG_FUNCTION
    std::vector<vk::PhysicalDevice> gpus = instance.enumeratePhysicalDevices();
    std::multimap<std::uint32_t, vk::PhysicalDevice> ratedGpus;

    for (const auto &i: gpus) {
        const auto suitable = isDeviceSuitable(i, surface, deviceExtensions);
        const auto deviceProperties = i.getProperties();
        if (suitable) {
            ratedGpus.insert(std::make_pair(rateDeviceSuitability(i), i));
            logger.debug("Physical Device") << deviceProperties.deviceName << " is a suitable GPU.";
        } else {
            logger.debug("Physical Device") << deviceProperties.deviceName << " is not a suitable GPU";
        }
    }
    if (!ratedGpus.empty() && ratedGpus.rbegin()->first > 0) {
        physical_device = ratedGpus.rbegin()->second;

    } else {
        throw VulkanBaseError("failed to find a suitable GPU!");
    }

    logger.info("Physical Device") << "Device extensions: " << deviceExtensions;
    const auto deviceProperties = physical_device.getProperties();
    logger.info("Physical Device") << vk::to_string(deviceProperties.deviceType) << ": " << deviceProperties.deviceName;

    depthFormat =
        vk_utils::findSupportedFormat(physical_device,
                                      {
                                          vk::Format::eD24UnormS8Uint,
                                          vk::Format::eD32Sfloat,
                                          vk::Format::eD32SfloatS8Uint,
                                      },
                                      vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    logger.info("Physical Device") << "Depth format: " << vk::to_string(depthFormat);

    maxMsaaSample = vk_utils::getMaxUsableSampleCount(physical_device);
    logger.info("Physical Device") << "MSAA Sample count: " << vk::to_string(maxMsaaSample);

    deviceFeature = physical_device.getFeatures();
    logger.info("Physical Device") << "multiDrawIndirect available: " << std::boolalpha
                                   << (deviceFeature.multiDrawIndirect == VK_TRUE);
}

void VulkanBase::createLogicalDevice(const std::vector<const char *> &deviceExtensions)
{
    DEBUG_FUNCTION
    float fQueuePriority = 1.0f;
    queueIndices = QueueFamilyIndices::findQueueFamilies(physical_device, surface);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies{queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value(),
                                           queueIndices.transferFamily.value()};

    std::ranges::transform(uniqueQueueFamilies, std::back_inserter(queueCreateInfos), [=](const auto &queueFamily) {
        return vk_init::populateDeviceQueueCreateInfo(1, queueFamily, fQueuePriority);
    });

    vk::PhysicalDeviceShaderDrawParametersFeatures shaderDrawParameters{
        .shaderDrawParameters = VK_TRUE,
    };

    vk::PhysicalDeviceFeatures deviceFeature{
        .multiDrawIndirect = this->deviceFeature.multiDrawIndirect,
        .drawIndirectFirstInstance = VK_TRUE,
        .fillModeNonSolid = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
    };
    vk::DeviceCreateInfo createInfo{
        .pNext = &shaderDrawParameters,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeature,
    };
    this->VulkanLoader::createLogicalDevice(physical_device, createInfo);
    baseDeletionQueue.push([&] { device.destroy(); });

    graphicsQueue = device.getQueue(queueIndices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(queueIndices.presentFamily.value(), 0);
}

void VulkanBase::createAllocator()
{
    DEBUG_FUNCTION
    vma::AllocatorCreateInfo allocatorInfo;
    allocatorInfo.physicalDevice = physical_device;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;

    allocator.init(allocatorInfo);
    baseDeletionQueue.push([&] { allocator.destroy(); });
}

}    // namespace pivot::graphics

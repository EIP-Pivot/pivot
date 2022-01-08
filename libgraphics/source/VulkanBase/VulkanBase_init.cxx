#include "pivot/graphics/VulkanBase.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/VulkanException.hxx"
#include "pivot/graphics/Window.hxx"
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
    for (auto &i: instanceExtensions) { extensions.push_back(i); }
    if (bEnableValidationLayers) { extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

    vk::ApplicationInfo applicationInfo{
        .pEngineName = "Pivot",
        .apiVersion = VK_API_VERSION_1_2,
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
    PRINT_STRING_ARRAY(logger->info("Instance"), "Instance extensions: ", extensions);
    this->VulkanLoader::createInstance(createInfo);
    baseDeletionQueue.push([&] { instance.destroy(); });
}

void VulkanBase::createDebugMessenger()
{
    DEBUG_FUNCTION
    if (!bEnableValidationLayers) return;
    auto debugInfo = vk_init::populateDebugUtilsMessengerCreateInfoEXT(&VulkanBase::debugCallback);
    debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(debugInfo);

    logger->warn("Validation Layers") << "Validation Layers are activated !";
    LOGGER_ENDL;
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
        if (isDeviceSuitable(i, surface, deviceExtensions)) {
            ratedGpus.insert(std::make_pair(rateDeviceSuitability(i), i));
        }
    }
    if (ratedGpus.rbegin()->first > 0) {
        physical_device = ratedGpus.rbegin()->second;
        maxMsaaSample = pivot::graphics::vk_utils::getMexUsableSampleCount(physical_device);
    } else {
        throw VulkanException("failed to find a suitable GPU!");
    }

    PRINT_STRING_ARRAY(logger->info("Physical Device"), "Device extensions", deviceExtensions);
    const auto deviceProperties = physical_device.getProperties();
    logger->info("Physical Device") << vk::to_string(deviceProperties.deviceType) << ": "
                                    << deviceProperties.deviceName;
    LOGGER_ENDL;

    deviceFeature = physical_device.getFeatures();
    logger->info("Physical Device") << "multiDrawIndirect available: " << std::boolalpha
                                    << (deviceFeature.multiDrawIndirect == VK_TRUE);
    LOGGER_ENDL;
}

void VulkanBase::createLogicalDevice(const std::vector<const char *> &deviceExtensions)
{
    DEBUG_FUNCTION
    float fQueuePriority = 1.0f;
    queueIndices = QueueFamilyIndices::findQueueFamilies(physical_device, surface);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies{queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value(),
                                           queueIndices.transferFamily.value()};

    for (const uint32_t queueFamily: uniqueQueueFamilies) {
        queueCreateInfos.push_back(vk_init::populateDeviceQueueCreateInfo(1, queueFamily, fQueuePriority));
    }

    vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndex{
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };
    vk::PhysicalDeviceVulkan11Features v11Features{
        .pNext = &descriptorIndex,
        .shaderDrawParameters = VK_TRUE,
    };

    vk::PhysicalDeviceFeatures deviceFeature{
        .multiDrawIndirect = this->deviceFeature.multiDrawIndirect,
        .drawIndirectFirstInstance = VK_TRUE,
        .fillModeNonSolid = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
    };
    vk::DeviceCreateInfo createInfo{
        .pNext = &v11Features,
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
    allocatorInfo.vulkanApiVersion = 0;
    allocatorInfo.frameInUseCount = MaxFrameInFlight;

    allocator = vma::createAllocator(allocatorInfo);
    baseDeletionQueue.push([&] { allocator.destroy(); });
}

}    // namespace pivot::graphics
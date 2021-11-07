#include "pivot/graphics/VulkanBase.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/VulkanException.hxx"
#include "pivot/graphics/common.hxx"
#include "pivot/graphics/vk_init.hxx"

#include <Logger.hpp>

#include <map>
#include <ranges>
#include <set>

namespace pivot::graphics
{

void VulkanBase::createInstance()
{
    DEBUG_FUNCTION
    auto debugInfo = vk_init::populateDebugUtilsMessengerCreateInfoEXT(&VulkanBase::debugCallback);
    auto extensions = Window::getRequiredExtensions();
    extensions.insert(extensions.end(), instanceExtensions.begin(), instanceExtensions.end());
    if (bEnableValidationLayers) { extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

    vk::ApplicationInfo applicationInfo{
        .pEngineName = "pivot",
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

    auto &l = logger->debug(__PRETTY_FUNCTION__);
    l << "Instance extensions : ";
    for (const auto &i: extensions) { l << i << " "; }
    LOGGER_ENDL;

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

void VulkanBase::pickPhysicalDevice()
{
    DEBUG_FUNCTION
    std::vector<vk::PhysicalDevice> gpus = instance.enumeratePhysicalDevices();
    std::multimap<uint32_t, vk::PhysicalDevice> ratedGpus;

    for (const auto &i:
         gpus | std::views::filter([&](const vk::PhysicalDevice &gpu) { return isDeviceSuitable(gpu, surface); })) {
        ratedGpus.insert(std::make_pair(rateDeviceSuitability(i), i));
    }
    if (ratedGpus.rbegin()->first > 0) {
        physical_device = ratedGpus.rbegin()->second;
    } else {
        throw VulkanException("failed to find a suitable GPU!");
    }

    const auto deviceProperties = physical_device.getProperties();
    logger->info(vk::to_string(deviceProperties.deviceType)) << deviceProperties.deviceName;
    LOGGER_ENDL;
}

void VulkanBase::createSurface()
{
    DEBUG_FUNCTION
    surface = window.createSurface(instance);
    baseDeletionQueue.push([&] { instance.destroy(surface); });
}

void VulkanBase::createLogicalDevice()
{
    DEBUG_FUNCTION
    float fQueuePriority = 1.0f;
    auto indices = QueueFamilyIndices::findQueueFamilies(physical_device, surface);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies{indices.graphicsFamily.value(), indices.presentFamily.value()};

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

    auto &l = logger->debug(__PRETTY_FUNCTION__);
    l << "Device extensions : ";
    for (const auto &i: deviceExtensions) { l << i << " "; }
    LOGGER_ENDL;

    this->VulkanLoader::createLogicalDevice(physical_device, createInfo);
    baseDeletionQueue.push([&] { device.destroy(); });

    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);
}

void VulkanBase::createAllocator()
{
    DEBUG_FUNCTION
    vma::AllocatorCreateInfo allocatorInfo;
    allocatorInfo.physicalDevice = physical_device;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.vulkanApiVersion = 0;
    allocatorInfo.frameInUseCount = PIVOT_MAX_FRAME_FRAME_IN_FLIGHT;

    allocator = vma::createAllocator(allocatorInfo);
    baseDeletionQueue.push([&] { allocator.destroy(); });
}
}    // namespace pivot::graphics
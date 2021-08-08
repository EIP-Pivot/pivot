#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <set>

VulkanApplication::VulkanApplication() {}

VulkanApplication::~VulkanApplication()
{
    if (device != VK_NULL_HANDLE) vkDeviceWaitIdle(device);
    swapchain.destroy();
    swapchainDeletionQueue.flush();
    mainDeletionQueue.flush();
}

void VulkanApplication::initBaseRessources()
{
    createInstance();
    createDebugMessenger();
}

void VulkanApplication::initVisualRessources()
{
    pickPhysicalDevice();
    createLogicalDevice();
    createSurface();
}

void VulkanApplication::createInstance()
{
    if (bEnableValidationLayers && !checkValiationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    auto debugInfo = vk_init::populateDebugUtilsMessengerCreateInfoEXT(&VulkanApplication::debugCallback);
    auto extensions = getWindowRequiredExtensions();
    if (bEnableValidationLayers) { extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

    VkApplicationInfo applicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_2,
    };
    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &applicationInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    if (bEnableValidationLayers) {
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    this->VulkanLoader::createInstance(createInfo);
}

void VulkanApplication::createDebugMessenger()
{
    if (!bEnableValidationLayers) return;

    auto debugInfo = vk_init::populateDebugUtilsMessengerCreateInfoEXT(&VulkanApplication::debugCallback);
    VK_TRY(vkCreateDebugUtilsMessengerEXT(instance, &debugInfo, nullptr, &debugUtilsMessenger));

    mainDeletionQueue.push([&] { vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr); });
}

void VulkanApplication::pickPhysicalDevice()
{
    for (const auto &device: vk_utils::getPhysicalDevices(instance)) {
        if (isDeviceSuitable(device, surface)) {
            physicalDevice = device;
            // maxMsaaSample = getMexUsableSampleCount(physicalDevice);
            break;
        }
    }
    if (physicalDevice == VK_NULL_HANDLE) {
        throw VulkanException("failed to find suitable GPU");
    } else {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        logger->info(vk_utils::tools::physicalDeviceTypeString(deviceProperties.deviceType))
            << deviceProperties.deviceName;
        LOGGER_ENDL;
    }
}

void VulkanApplication::createLogicalDevice()
{
    float fQueuePriority = 1.0f;
    auto indices = QueueFamilyIndices::findQueueFamilies(physicalDevice, surface);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies{indices.graphicsFamily.value(), indices.presentFamily.value()};

    for (const uint32_t queueFamily: uniqueQueueFamilies) {
        queueCreateInfos.push_back(vk_init::populateDeviceQueueCreateInfo(1, queueFamily, fQueuePriority));
    }

    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndex{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .pNext = nullptr,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };
    VkPhysicalDeviceVulkan11Features v11Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &descriptorIndex,
        .shaderDrawParameters = VK_TRUE,
    };

    VkPhysicalDeviceFeatures deviceFeature{
        .fillModeNonSolid = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
    };
    VkDeviceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &v11Features,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeature,
    };
    VK_TRY(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));
    mainDeletionQueue.push([=] { vkDestroyDevice(device, nullptr); });

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanApplication::createAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo{
        .physicalDevice = physicalDevice,
        .device = device,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_2,
    };
    VK_TRY(vmaCreateAllocator(&allocatorInfo, &allocator));
    mainDeletionQueue.push([&] { vmaDestroyAllocator(allocator); });
}

void VulkanApplication::createSurface()
{
    window->get().createSurface(instance, &surface);
    mainDeletionQueue.push([&] { vkDestroySurfaceKHR(instance, surface, nullptr); });
}
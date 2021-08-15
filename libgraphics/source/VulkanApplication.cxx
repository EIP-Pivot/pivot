#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/Camera.hxx"
#include "pivot/graphics/PipelineBuilder.hxx"
#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/types/Material.hpp"
#include "pivot/graphics/types/UniformBufferObject.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <numeric>
#include <set>

#define MAX_OBJECT 1000
#define MAX_TEXTURES 1000

#define MAX_COMMANDS 100
#define MAX_MATERIALS 100
VulkanApplication::VulkanApplication() {}

VulkanApplication::~VulkanApplication()
{
    if (device != VK_NULL_HANDLE) vkDeviceWaitIdle(device);
    swapchain.destroy();
    swapchainDeletionQueue.flush();
    mainDeletionQueue.flush();
}

void VulkanApplication::initVulkanRessources()
{
    createInstance();
    createDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();

    swapchain.init(window->get().getSize(), physicalDevice, device, surface);

    createAllocator();
    createSyncStructure();
    createRenderPass();
    createDescriptorSetsLayout();
    createPipeline();
    createCommandPool();
    createDepthResources();
    createColorResources();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();

    this->pushModelsToGPU();
    this->pushTexturesToGPU();

    createTextureSampler();
    createTextureDescriptorSets();
    createCommandBuffers();
}

void VulkanApplication::createInstance()
{
    if (bEnableValidationLayers && !checkValiationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    auto debugInfo = vk_init::populateDebugUtilsMessengerCreateInfoEXT(&VulkanApplication::debugCallback);
    auto extensions = window->get().getWindowRequiredExtensions();
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

void VulkanApplication::pickPhysicalDevice()
{
    for (const auto &device: vk_utils::getPhysicalDevices(instance)) {
        if (isDeviceSuitable(device, surface)) {
            physicalDevice = device;
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

void VulkanApplication::createDescriptorSetsLayout()
{
    std::vector<VkDescriptorBindingFlags> textureBindingFlags{
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
    };
    VkDescriptorSetLayoutBindingFlagsCreateInfo textureBindingInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .pNext = nullptr,
        .bindingCount = static_cast<uint32_t>(textureBindingFlags.size()),
        .pBindingFlags = textureBindingFlags.data(),
    };
    VkDescriptorSetLayoutBinding samplerLayoutBiding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 32,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,
    };
    VkDescriptorSetLayoutCreateInfo texturesSetLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &textureBindingInfo,
        .bindingCount = 1,
        .pBindings = &samplerLayoutBiding,
    };

    std::array<VkDescriptorSetLayoutBinding, 2> layoutBinding;
    layoutBinding.at(0) = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr,
    };
    layoutBinding.at(1) = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .bindingCount = static_cast<uint32_t>(layoutBinding.size()),
        .pBindings = layoutBinding.data(),
    };
    VK_TRY(vkCreateDescriptorSetLayout(device, &texturesSetLayoutInfo, nullptr, &texturesSetLayout));
    VK_TRY(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

    mainDeletionQueue.push([&] {
        vkDestroyDescriptorSetLayout(device, texturesSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    });
}

void VulkanApplication::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize[] = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAME_FRAME_IN_FLIGHT,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 100,
        },
    };

    VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .maxSets = static_cast<uint32_t>(std::accumulate(poolSize, poolSize + std::size(poolSize), 0,
                                                         [](auto prev, auto &i) { return prev + i.descriptorCount; })),
        .poolSizeCount = std::size(poolSize),
        .pPoolSizes = poolSize,
    };
    VK_TRY(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));
    swapchainDeletionQueue.push([&]() { vkDestroyDescriptorPool(device, descriptorPool, nullptr); });
}

void VulkanApplication::createDescriptorSets()
{
    for (auto &f: frames) {
        VkDescriptorSetAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayout,
        };
        VK_TRY(vkAllocateDescriptorSets(device, &allocInfo, &f.data.objectDescriptor));

        VkDescriptorBufferInfo bufferInfo{
            .buffer = f.data.uniformBuffers.buffer,
            .offset = 0,
            .range = sizeof(gpuObject::UniformBufferObject) * MAX_OBJECT,
        };
        VkDescriptorBufferInfo materialInfo{
            .buffer = f.data.materialBuffer.buffer,
            .offset = 0,
            .range = sizeof(gpuObject::Material) * MAX_MATERIALS,
        };
        std::vector<VkWriteDescriptorSet> descriptorWrites{
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = f.data.objectDescriptor,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &bufferInfo,
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = f.data.objectDescriptor,
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &materialInfo,
            },
        };
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                               nullptr);
    }
}

void VulkanApplication::createTextureDescriptorSets()
{
    std::vector<VkDescriptorImageInfo> imagesInfos;
    for (auto &[_, t]: loadedTextures) {
        imagesInfos.push_back({
            .sampler = textureSampler,
            .imageView = t.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
    }

    uint32_t counts[] = {static_cast<uint32_t>(imagesInfos.size())};
    VkDescriptorSetVariableDescriptorCountAllocateInfo set_counts{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorSetCount = std::size(counts),
        .pDescriptorCounts = counts,
    };
    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &set_counts,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &texturesSetLayout,
    };
    VK_TRY(vkAllocateDescriptorSets(device, &allocInfo, &texturesSet));
    VkWriteDescriptorSet descriptorWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = texturesSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = static_cast<uint32_t>(imagesInfos.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = imagesInfos.data(),
    };
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void VulkanApplication::createUniformBuffers()
{
    for (auto &f: frames) {
        f.data.uniformBuffers = createBuffer(sizeof(gpuObject::UniformBufferObject) * MAX_OBJECT,
                                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        f.data.materialBuffer = createBuffer(sizeof(gpuObject::Material) * MAX_MATERIALS,
                                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    }
    mainDeletionQueue.push([&] {
        for (auto &f: frames) {
            vmaDestroyBuffer(allocator, f.data.uniformBuffers.buffer, f.data.uniformBuffers.memory);
            vmaDestroyBuffer(allocator, f.data.materialBuffer.buffer, f.data.materialBuffer.memory);
        }
    });
}

void VulkanApplication::createDepthResources()
{
    VkFormat depthFormat = vk_utils::findSupportedFormat(
        physicalDevice, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depthFormat,
        .extent = swapchain.getSwapchainExtent3D(),
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_2_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    VmaAllocationCreateInfo allocInfo{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };
    VK_TRY(vmaCreateImage(allocator, &imageInfo, &allocInfo, &depthResources.image, &depthResources.memory, nullptr));

    auto createInfo = vk_init::populateVkImageViewCreateInfo(depthResources.image, depthFormat);
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    VK_TRY(vkCreateImageView(device, &createInfo, nullptr, &depthResources.imageView));

    transitionImageLayout(depthResources.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    swapchainDeletionQueue.push([&] {
        vkDestroyImageView(device, depthResources.imageView, nullptr);
        vmaDestroyImage(allocator, depthResources.image, depthResources.memory);
    });
}

void VulkanApplication::createColorResources()
{
    VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = swapchain.getSwapchainFormat(),
        .extent = swapchain.getSwapchainExtent3D(),
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_2_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    VmaAllocationCreateInfo allocInfo{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };
    VK_TRY(vmaCreateImage(allocator, &imageInfo, &allocInfo, &colorImage.image, &colorImage.memory, nullptr));

    auto createInfo = vk_init::populateVkImageViewCreateInfo(colorImage.image, swapchain.getSwapchainFormat());
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VK_TRY(vkCreateImageView(device, &createInfo, nullptr, &colorImage.imageView));

    swapchainDeletionQueue.push([&] {
        vkDestroyImageView(device, colorImage.imageView, nullptr);
        vmaDestroyImage(allocator, colorImage.image, colorImage.memory);
    });
}

void VulkanApplication::createRenderPass()
{
    VkAttachmentDescription colorAttachment{
        .format = swapchain.getSwapchainFormat(),
        .samples = VK_SAMPLE_COUNT_2_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentDescription depthAttachment{
        .format = vk_utils::findSupportedFormat(
            physicalDevice, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
        .samples = VK_SAMPLE_COUNT_2_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentDescription colorAttachmentResolve{
        .format = swapchain.getSwapchainFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentReference depthAttachmentRef{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentReference colorAttachmentResolveRef{
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pResolveAttachments = &colorAttachmentResolveRef,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };
    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };
    VK_TRY(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
    swapchainDeletionQueue.push([&] { vkDestroyRenderPass(device, renderPass, nullptr); });
}
void VulkanApplication::createFramebuffers()
{
    swapChainFramebuffers.resize(swapchain.nbOfImage());
    for (size_t i = 0; i < swapchain.nbOfImage(); i++) {
        std::array<VkImageView, 3> attachments = {colorImage.imageView, depthResources.imageView,
                                                  swapchain.getSwapchainImageView(i)};

        VkFramebufferCreateInfo framebufferInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = swapchain.getSwapchainExtent().width,
            .height = swapchain.getSwapchainExtent().height,
            .layers = 1,
        };

        VK_TRY(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]));
    }
    swapchainDeletionQueue.push([&] {
        for (auto &framebuffer: swapChainFramebuffers) { vkDestroyFramebuffer(device, framebuffer, nullptr); }
    });
}

void VulkanApplication::createSyncStructure()
{
    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
    };
    VkFenceCreateInfo fenceInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    VkFenceCreateInfo uploadFenceInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    VK_TRY(vkCreateFence(device, &uploadFenceInfo, nullptr, &uploadContext.uploadFence));
    for (auto &f: frames) {
        VK_TRY(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &f.imageAvailableSemaphore));
        VK_TRY(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &f.renderFinishedSemaphore));
        VK_TRY(vkCreateFence(device, &fenceInfo, nullptr, &f.inFlightFences));
    }

    mainDeletionQueue.push([&] {
        vkDestroyFence(device, uploadContext.uploadFence, nullptr);
        for (auto &f: frames) {
            vkDestroyFence(device, f.inFlightFences, nullptr);
            vkDestroySemaphore(device, f.renderFinishedSemaphore, nullptr);
            vkDestroySemaphore(device, f.imageAvailableSemaphore, nullptr);
        }
    });
}

void VulkanApplication::createCommandPool()
{
    auto indices = QueueFamilyIndices::findQueueFamilies(physicalDevice, surface);
    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphicsFamily.value(),
    };
    VK_TRY(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
    VK_TRY(vkCreateCommandPool(device, &poolInfo, nullptr, &uploadContext.commandPool));
    mainDeletionQueue.push([&] {
        vkDestroyCommandPool(device, uploadContext.commandPool, nullptr);
        vkDestroyCommandPool(device, commandPool, nullptr);
    });
}

void VulkanApplication::createCommandBuffers()
{
    commandBuffers.resize(swapchain.nbOfImage());

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
    };
    VK_TRY(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()));
    swapchainDeletionQueue.push(
        [&]() { vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data()); });
}

void VulkanApplication::createTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = static_cast<float>(mipLevels),
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    VK_TRY(vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler));
    mainDeletionQueue.push([&] { vkDestroySampler(device, textureSampler, nullptr); });
}

void VulkanApplication::createPipeline()
{
    auto vertShaderCode = vk_utils::readFile("shaders/triangle.vert.spv");
    auto fragShaderCode = vk_utils::readFile("shaders/triangle.frag.spv");

    auto vertShaderModule = vk_utils::createShaderModule(device, vertShaderCode);
    auto fragShaderModule = vk_utils::createShaderModule(device, fragShaderCode);

    std::vector<VkPushConstantRange> pipelinePushConstant = {
        vk_init::populateVkPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                             sizeof(Camera::GPUCameraData)),
    };
    std::vector<VkDescriptorSetLayout> setLayout = {descriptorSetLayout, texturesSetLayout};
    auto pipelineLayoutCreateInfo = vk_init::populateVkPipelineLayoutCreateInfo(setLayout, pipelinePushConstant);
    VK_TRY(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

    std::vector<VkVertexInputBindingDescription> binding = {Vertex::getBindingDescription()};
    std::vector<VkVertexInputAttributeDescription> attribute = Vertex::getAttributeDescriptons();

    PipelineBuilder builder;
    builder.pipelineLayout = pipelineLayout;
    builder.shaderStages.push_back(
        vk_init::populateVkPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule));
    builder.shaderStages.push_back(
        vk_init::populateVkPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule));
    builder.vertexInputInfo = vk_init::populateVkPipelineVertexInputStateCreateInfo(binding, attribute);
    builder.inputAssembly =
        vk_init::populateVkPipelineInputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
    builder.multisampling = vk_init::populateVkPipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_2_BIT);
    builder.depthStencil = vk_init::populateVkPipelineDepthStencilStateCreateInfo();
    builder.viewport.x = 0.0f;
    builder.viewport.y = 0.0f;
    builder.viewport.width = swapchain.getSwapchainExtent().width;
    builder.viewport.height = swapchain.getSwapchainExtent().height;
    builder.viewport.minDepth = 0.0f;
    builder.viewport.maxDepth = 1.0f;
    builder.scissor.offset = {0, 0};
    builder.scissor.extent = swapchain.getSwapchainExtent();
    builder.colorBlendAttachment = vk_init::populateVkPipelineColorBlendAttachmentState();
    builder.rasterizer = vk_init::populateVkPipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
    builder.rasterizer.cullMode = VK_CULL_MODE_NONE;
    builder.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    graphicsPipeline = builder.build(device, renderPass);

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    swapchainDeletionQueue.push([&] {
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    });
}
#include "pivot/graphics/vk_init.hxx"

VkDebugUtilsMessengerCreateInfoEXT vk_init::populateDebugUtilsMessengerCreateInfoEXT(VKAPI_ATTR VkBool32 VKAPI_CALL (
    *debugMessageFunc)(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
                       const VkDebugUtilsMessengerCallbackDataEXT *, void *))
{

    VkDebugUtilsMessageSeverityFlagsEXT severityFlags(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
    VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT);
    return VkDebugUtilsMessengerCreateInfoEXT{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = debugMessageFunc,
    };
}

VkDeviceQueueCreateInfo vk_init::populateDeviceQueueCreateInfo(const uint32_t count, const uint32_t queue,
                                                               const float &priority)
{
    return VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .queueFamilyIndex = queue,
        .queueCount = count,
        .pQueuePriorities = &priority,
    };
}

VkImageViewCreateInfo vk_init::populateVkImageViewCreateInfo(VkImage &img, VkFormat format, uint32_t mipLevel)
{
    return VkImageViewCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .image = img,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = mipLevel,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
}

VkShaderModuleCreateInfo vk_init::populateVkShaderModuleCreateInfo(const std::vector<std::byte> &code)
{
    return VkShaderModuleCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };
}

VkPipelineShaderStageCreateInfo vk_init::populateVkPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
                                                                                 VkShaderModule &module,
                                                                                 const char *entryPoint)
{
    return VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = stage,
        .module = module,
        .pName = entryPoint,
    };
}

VkPipelineVertexInputStateCreateInfo
vk_init::populateVkPipelineVertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription> &binding,
                                                      const std::vector<VkVertexInputAttributeDescription> &attribute)
{
    return VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = static_cast<uint32_t>(binding.size()),
        .pVertexBindingDescriptions = binding.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute.size()),
        .pVertexAttributeDescriptions = attribute.data(),
    };
}
VkPipelineInputAssemblyStateCreateInfo vk_init::populateVkPipelineInputAssemblyCreateInfo(VkPrimitiveTopology topology,
                                                                                          VkBool32 restart)
{
    return VkPipelineInputAssemblyStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = topology,
        .primitiveRestartEnable = restart,
    };
}

VkPipelineRasterizationStateCreateInfo vk_init::populateVkPipelineRasterizationStateCreateInfo(VkPolygonMode mode)
{
    return VkPipelineRasterizationStateCreateInfo{

        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = mode,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };
}

VkPipelineMultisampleStateCreateInfo
vk_init::populateVkPipelineMultisampleStateCreateInfo(VkSampleCountFlagBits msaaSamples)
{
    return VkPipelineMultisampleStateCreateInfo{

        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = msaaSamples,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,

    };
}

VkPipelineColorBlendAttachmentState vk_init::populateVkPipelineColorBlendAttachmentState()
{
    return VkPipelineColorBlendAttachmentState{
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
}

VkPipelineLayoutCreateInfo
vk_init::populateVkPipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout> &setLayout,
                                            const std::vector<VkPushConstantRange> &pushLayout)
{
    auto createInfo = empty::populateVkPipelineLayoutCreateInfo();

    createInfo.setLayoutCount = static_cast<uint32_t>(setLayout.size());
    createInfo.pSetLayouts = setLayout.data();
    createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushLayout.size());
    createInfo.pPushConstantRanges = pushLayout.data();
    return createInfo;
}

VkPipelineDepthStencilStateCreateInfo vk_init::populateVkPipelineDepthStencilStateCreateInfo()
{
    return VkPipelineDepthStencilStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,

    };
}

VkPushConstantRange vk_init::populateVkPushConstantRange(VkShaderStageFlags stage, uint32_t size, uint32_t offset)
{
    VkPushConstantRange push_constant{};
    push_constant.offset = offset;
    push_constant.size = size;
    push_constant.stageFlags = stage;
    return push_constant;
}

VkPipelineLayoutCreateInfo vk_init::empty::populateVkPipelineLayoutCreateInfo()
{
    return VkPipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };
}

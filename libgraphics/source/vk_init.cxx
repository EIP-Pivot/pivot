#include "pivot/graphics/vk_init.hxx"

vk::DebugUtilsMessengerCreateInfoEXT
vk_init::populateDebugUtilsMessengerCreateInfoEXT(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
{
    return vk::DebugUtilsMessengerCreateInfoEXT{
        .messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = debugCallback,
    };
}

vk::DeviceQueueCreateInfo vk_init::populateDeviceQueueCreateInfo(const uint32_t count, const uint32_t queue,
                                                                 const float &priority)
{
    return vk::DeviceQueueCreateInfo{
        .queueFamilyIndex = queue,
        .queueCount = count,
        .pQueuePriorities = &priority,
    };
}

vk::ImageViewCreateInfo vk_init::populateVkImageViewCreateInfo(vk::Image &img, vk::Format format, uint32_t mipLevel)
{
    return vk::ImageViewCreateInfo{
        .image = img,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .components =
            {
                .r = vk::ComponentSwizzle::eIdentity,
                .g = vk::ComponentSwizzle::eIdentity,
                .b = vk::ComponentSwizzle::eIdentity,
                .a = vk::ComponentSwizzle::eIdentity,
            },
        .subresourceRange =
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = mipLevel,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
}

vk::ShaderModuleCreateInfo vk_init::populateVkShaderModuleCreateInfo(std::span<const std::byte> code)
{
    return vk::ShaderModuleCreateInfo{
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };
}

vk::PipelineShaderStageCreateInfo vk_init::populateVkPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits stage,
                                                                                   vk::ShaderModule &module,
                                                                                   const char *entryPoint)
{
    return vk::PipelineShaderStageCreateInfo{
        .stage = stage,
        .module = module,
        .pName = entryPoint,
    };
}

vk::PipelineVertexInputStateCreateInfo
vk_init::populateVkPipelineVertexInputStateCreateInfo(std::span<const vk::VertexInputBindingDescription> binding,
                                                      std::span<const vk::VertexInputAttributeDescription> attribute)
{
    return vk::PipelineVertexInputStateCreateInfo{
        .vertexBindingDescriptionCount = static_cast<uint32_t>(binding.size()),
        .pVertexBindingDescriptions = binding.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute.size()),
        .pVertexAttributeDescriptions = attribute.data(),
    };
}
vk::PipelineInputAssemblyStateCreateInfo
vk_init::populateVkPipelineInputAssemblyCreateInfo(vk::PrimitiveTopology topology, vk::Bool32 restart)
{
    return vk::PipelineInputAssemblyStateCreateInfo{
        .topology = topology,
        .primitiveRestartEnable = restart,
    };
}

vk::PipelineRasterizationStateCreateInfo vk_init::populateVkPipelineRasterizationStateCreateInfo(vk::PolygonMode mode)
{
    return vk::PipelineRasterizationStateCreateInfo{
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = mode,
        .cullMode = vk::CullModeFlagBits::eNone,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };
}

vk::PipelineMultisampleStateCreateInfo
vk_init::populateVkPipelineMultisampleStateCreateInfo(vk::SampleCountFlagBits msaaSamples)
{
    return vk::PipelineMultisampleStateCreateInfo{
        .rasterizationSamples = msaaSamples,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,

    };
}

vk::PipelineColorBlendAttachmentState vk_init::populateVkPipelineColorBlendAttachmentState()
{
    return vk::PipelineColorBlendAttachmentState{
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,

    };
}

vk::PipelineLayoutCreateInfo
vk_init::populateVkPipelineLayoutCreateInfo(std::span<const vk::DescriptorSetLayout> setLayout,
                                            std::span<const vk::PushConstantRange> pushLayout)
{
    vk::PipelineLayoutCreateInfo createInfo;

    createInfo.setSetLayouts(setLayout);
    createInfo.setPushConstantRanges(pushLayout);
    return createInfo;
}

vk::PipelineDepthStencilStateCreateInfo vk_init::populateVkPipelineDepthStencilStateCreateInfo()
{
    return vk::PipelineDepthStencilStateCreateInfo{
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,

    };
}

vk::PushConstantRange vk_init::populateVkPushConstantRange(vk::ShaderStageFlags stage, uint32_t size, uint32_t offset)
{
    return vk::PushConstantRange{
        .stageFlags = stage,
        .offset = offset,
        .size = size,
    };
}

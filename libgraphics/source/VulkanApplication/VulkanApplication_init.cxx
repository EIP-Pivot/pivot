#include "pivot/graphics/VulkanApplication.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/PipelineBuilders/ComputePipelineBuilder.hxx"
#include "pivot/graphics/PipelineBuilders/GraphicsPipelineBuilder.hxx"
#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/types/Material.hxx"
#include "pivot/graphics/types/UniformBufferObject.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <map>
#include <numeric>
#include <set>

namespace pivot::graphics
{

void VulkanApplication::createRenderPass()
{
    DEBUG_FUNCTION
    vk::AttachmentDescription colorAttachment{
        .format = swapchain.getSwapchainFormat(),
        .samples = maxMsaaSample,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eColorAttachmentOptimal,
    };
    vk::AttachmentDescription depthAttachment{
        .format = pivot::graphics::vk_utils::findSupportedFormat(
            physical_device, {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment),
        .samples = maxMsaaSample,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
    };
    vk::AttachmentDescription colorAttachmentResolve{
        .format = swapchain.getSwapchainFormat(),
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eDontCare,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,
    };
    vk::AttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal,
    };
    vk::AttachmentReference depthAttachmentRef{
        .attachment = 1,
        .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
    };
    vk::AttachmentReference colorAttachmentResolveRef{
        .attachment = 2,
        .layout = vk::ImageLayout::eColorAttachmentOptimal,
    };
    vk::SubpassDescription subpass{
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pResolveAttachments = &colorAttachmentResolveRef,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };
    vk::SubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .dstStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .srcAccessMask = vk::AccessFlagBits::eNoneKHR,
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
    };

    std::array<vk::AttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    vk::RenderPassCreateInfo renderPassInfo{
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };
    renderPass = device.createRenderPass(renderPassInfo);

    swapchainDeletionQueue.push([&] { device.destroy(renderPass); });
}

void VulkanApplication::createPipelineCache()
{
    DEBUG_FUNCTION
    vk::PipelineCacheCreateInfo createInfo{};
    pipelineCache = device.createPipelineCache(createInfo);
    mainDeletionQueue.push([&] { device.destroy(pipelineCache); });
}

void VulkanApplication::createPipelineLayout()
{
    DEBUG_FUNCTION
    std::vector<vk::PushConstantRange> pipelinePushConstant = {
        vk_init::populateVkPushConstantRange(vk::ShaderStageFlagBits::eVertex, sizeof(gpu_object::VertexPushConstant)),
        vk_init::populateVkPushConstantRange(vk::ShaderStageFlagBits::eFragment,
                                             sizeof(gpu_object::FragmentPushConstant),
                                             sizeof(gpu_object::VertexPushConstant)),
    };

    std::vector<vk::DescriptorSetLayout> setLayout = {drawResolver.getDescriptorSetLayout(), ressourcesSetLayout};
    auto pipelineLayoutCreateInfo = vk_init::populateVkPipelineLayoutCreateInfo(setLayout, pipelinePushConstant);
    pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
    mainDeletionQueue.push([&] { device.destroy(pipelineLayout); });
}

void VulkanApplication::createCullingPipelineLayout()
{
    DEBUG_FUNCTION
    std::vector<vk::PushConstantRange> pipelinePushConstant = {vk_init::populateVkPushConstantRange(
        vk::ShaderStageFlagBits::eCompute, sizeof(gpu_object::CullingPushConstant))};
    std::vector<vk::DescriptorSetLayout> setLayout = {drawResolver.getDescriptorSetLayout(), ressourcesSetLayout};
    auto pipelineLayoutCreateInfo = vk_init::populateVkPipelineLayoutCreateInfo(setLayout, pipelinePushConstant);
    cullingLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
    mainDeletionQueue.push([&] { device.destroy(cullingLayout); });
}

void VulkanApplication::createPipeline()
{
    DEBUG_FUNCTION

    pivot::graphics::GraphicsPipelineBuilder builder(swapchain.getSwapchainExtent());
    builder.setPipelineLayout(pipelineLayout)
        .setRenderPass(renderPass)
        .setMsaaSample(maxMsaaSample)
        .setFaceCulling(vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise)
        .setVertexShaderPath("shaders/triangle.vert.spv")
        .setFragmentShaderPath("shaders/triangle.frag.spv");
    graphicsPipeline = builder.build(device, pipelineCache);
    pivot::graphics::vk_debug::setObjectName(device, graphicsPipeline, "Main graphics pipeline");

    swapchainDeletionQueue.push([&] { device.destroy(graphicsPipeline); });
}

void VulkanApplication::createCullingPipeline()
{
    DEBUG_FUNCTION
    pivot::graphics::ComputePipelineBuilder builder;
    builder.setPipelineLayout(cullingLayout).setComputeShaderPath("shaders/culling.comp.spv");
    cullingPipeline = builder.build(device, pipelineCache);
    pivot::graphics::vk_debug::setObjectName(device, cullingPipeline, "Culling compute pipeline");

    mainDeletionQueue.push([&] { device.destroyPipeline(cullingPipeline); });
}

void VulkanApplication::createFramebuffers()
{
    DEBUG_FUNCTION
    swapChainFramebuffers.resize(swapchain.nbOfImage());
    for (size_t i = 0; i < swapchain.nbOfImage(); i++) {
        std::array<vk::ImageView, 3> attachments = {colorImage.imageView, depthResources.imageView,
                                                    swapchain.getSwapchainImageView(i)};

        vk::FramebufferCreateInfo framebufferInfo{
            .renderPass = renderPass,
            .width = swapchain.getSwapchainExtent().width,
            .height = swapchain.getSwapchainExtent().height,
            .layers = 1,
        };
        framebufferInfo.setAttachments(attachments);

        swapChainFramebuffers.at(i) = device.createFramebuffer(framebufferInfo);
        pivot::graphics::vk_debug::setObjectName(device, swapChainFramebuffers.at(i),
                                                 "FrameBuffer " + std::to_string(i));
    }
    swapchainDeletionQueue.push([&] {
        for (auto &framebuffer: swapChainFramebuffers) { device.destroy(framebuffer); }
    });
}

void VulkanApplication::createCommandPool()
{
    DEBUG_FUNCTION
    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueIndices.graphicsFamily.value(),
    };
    commandPool = device.createCommandPool(poolInfo);
    pivot::graphics::vk_debug::setObjectName(device, commandPool, "Main Command Pool");
    imguiContext.cmdPool = device.createCommandPool(poolInfo);
    pivot::graphics::vk_debug::setObjectName(device, imguiContext.cmdPool, "ImGui Command Pool");
    mainDeletionQueue.push([&] {
        device.destroy(imguiContext.cmdPool);
        device.destroy(commandPool);
    });
}

void VulkanApplication::createCommandBuffers()
{
    DEBUG_FUNCTION
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(swapchain.nbOfImage()),
    };
    commandBuffersPrimary = device.allocateCommandBuffers(allocInfo);

    vk::CommandBufferAllocateInfo drawInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::eSecondary,
        .commandBufferCount = static_cast<uint32_t>(swapchain.nbOfImage()),
    };
    commandBuffersSecondary = device.allocateCommandBuffers(drawInfo);

    vk::CommandBufferAllocateInfo imguiInfo{
        .commandPool = imguiContext.cmdPool,
        .level = vk::CommandBufferLevel::eSecondary,
        .commandBufferCount = static_cast<uint32_t>(swapchain.nbOfImage()),
    };
    imguiContext.cmdBuffer = device.allocateCommandBuffers(imguiInfo);
    swapchainDeletionQueue.push([&] {
        device.free(commandPool, commandBuffersPrimary);
        device.free(commandPool, commandBuffersSecondary);
        device.free(imguiContext.cmdPool, imguiContext.cmdBuffer);
    });
}

void VulkanApplication::createSyncStructure()
{
    DEBUG_FUNCTION
    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled,
    };

    for (auto &f: frames) {
        f.imageAvailableSemaphore = device.createSemaphore(semaphoreInfo);
        f.renderFinishedSemaphore = device.createSemaphore(semaphoreInfo);
        f.inFlightFences = device.createFence(fenceInfo);
    }

    mainDeletionQueue.push([&] {
        for (auto &f: frames) {
            device.destroy(f.inFlightFences);
            device.destroy(f.renderFinishedSemaphore);
            device.destroy(f.imageAvailableSemaphore);
        }
    });
}

void VulkanApplication::createRessourcesDescriptorSetLayout()
{
    DEBUG_FUNCTION
    std::vector<vk::DescriptorBindingFlags> flags{
        {},
        {},
        vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound,
    };

    vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingInfo{
        .bindingCount = static_cast<uint32_t>(flags.size()),
        .pBindingFlags = flags.data(),
    };
    vk::DescriptorSetLayoutBinding boundingBoxBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eCompute,
    };
    vk::DescriptorSetLayoutBinding materialLayoutBinding{
        .binding = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
    };
    vk::DescriptorSetLayoutBinding samplerLayoutBiding{
        .binding = 2,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = static_cast<uint32_t>(assetStorage.getTextures().size()),
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
    };
    std::array<vk::DescriptorSetLayoutBinding, 3> bindings = {
        boundingBoxBinding,
        materialLayoutBinding,
        samplerLayoutBiding,
    };
    vk::DescriptorSetLayoutCreateInfo ressourcesSetLayoutInfo{
        .pNext = &bindingInfo,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };
    ressourcesSetLayout = device.createDescriptorSetLayout(ressourcesSetLayoutInfo);
    pivot::graphics::vk_debug::setObjectName(device, ressourcesSetLayout, "Ressources Set Layout");
    mainDeletionQueue.push([&] { device.destroy(ressourcesSetLayout); });
}

void VulkanApplication::createDescriptorPool()
{
    DEBUG_FUNCTION
    vk::DescriptorPoolSize poolSize[] = {
        {
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = static_cast<uint32_t>(assetStorage.getTextures().size()),
        },
        {
            .type = vk::DescriptorType::eStorageBuffer,
            .descriptorCount = assetStorage.getMaterialBuffer().size,
        },
    };

    vk::DescriptorPoolCreateInfo poolInfo{
        .maxSets = static_cast<uint32_t>(std::accumulate(poolSize, poolSize + std::size(poolSize), 0,
                                                         [](auto prev, auto &i) { return prev + i.descriptorCount; })),
        .poolSizeCount = std::size(poolSize),
        .pPoolSizes = poolSize,
    };
    descriptorPool = device.createDescriptorPool(poolInfo);
    pivot::graphics::vk_debug::setObjectName(device, descriptorPool, "Main Descriptor Pool");
    mainDeletionQueue.push([&] { device.destroyDescriptorPool(descriptorPool); });
}

void VulkanApplication::createRessourcesDescriptorSets()
{
    DEBUG_FUNCTION

    uint32_t counts[] = {static_cast<uint32_t>(assetStorage.getTextures().size())};
    vk::DescriptorSetVariableDescriptorCountAllocateInfo set_counts{
        .descriptorSetCount = std::size(counts),
        .pDescriptorCounts = counts,
    };
    vk::DescriptorSetAllocateInfo allocInfo{
        .pNext = &set_counts,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &ressourcesSetLayout,
    };
    ressourceDescriptorSet = device.allocateDescriptorSets(allocInfo).front();

    std::vector<vk::DescriptorImageInfo> imagesInfos;
    for (auto &t: assetStorage.getTextures().getStorage()) {
        imagesInfos.push_back({
            .sampler = textureSampler,
            .imageView = t.image.imageView,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        });
    }
    auto &materialBuffer = assetStorage.getMaterialBuffer();
    vk::DescriptorBufferInfo materialInfo{
        .buffer = materialBuffer.buffer,
        .offset = 0,
        .range = materialBuffer.size,
    };
    auto &boundingBoxBuffer = assetStorage.getBoundingBoxBuffer();
    vk::DescriptorBufferInfo boundingBoxInfo{
        .buffer = boundingBoxBuffer.buffer,
        .offset = 0,
        .range = boundingBoxBuffer.size,
    };
    std::vector<vk::WriteDescriptorSet> descriptorWrite{
        {
            .dstSet = ressourceDescriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &boundingBoxInfo,
        },
        {
            .dstSet = ressourceDescriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &materialInfo,
        },
        {
            .dstSet = ressourceDescriptorSet,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = static_cast<uint32_t>(imagesInfos.size()),
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = imagesInfos.data(),
        },
    };
    device.updateDescriptorSets(descriptorWrite, 0);
}

void VulkanApplication::createTextureSampler()
{
    DEBUG_FUNCTION
    vk::PhysicalDeviceProperties properties = physical_device.getProperties();

    vk::SamplerCreateInfo samplerInfo{
        .magFilter = vk::Filter::eNearest,
        .minFilter = vk::Filter::eNearest,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = 100,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = VK_FALSE,
    };
    textureSampler = device.createSampler(samplerInfo);
    pivot::graphics::vk_debug::setObjectName(device, textureSampler, "Texture Sampler");
    mainDeletionQueue.push([&] { device.destroySampler(textureSampler); });
}

void VulkanApplication::createDepthResources()
{
    DEBUG_FUNCTION
    vk::Format depthFormat = pivot::graphics::vk_utils::findSupportedFormat(
        physical_device, {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

    depthResources.size = swapchain.getSwapchainExtent3D();
    depthResources.format = depthFormat;
    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = depthFormat,
        .extent = swapchain.getSwapchainExtent3D(),
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = maxMsaaSample,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };
    vma::AllocationCreateInfo allocInfo;
    allocInfo.setUsage(vma::MemoryUsage::eGpuOnly);
    std::tie(depthResources.image, depthResources.memory) = allocator.createImage(imageInfo, allocInfo);

    auto createInfo = vk_init::populateVkImageViewCreateInfo(depthResources.image, depthFormat);
    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    depthResources.imageView = device.createImageView(createInfo);

    depthResources.transitionLayout(*this, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    pivot::graphics::vk_debug::setObjectName(device, depthResources.image, "Depth Image");
    pivot::graphics::vk_debug::setObjectName(device, depthResources.imageView, "Depth Image view");
    swapchainDeletionQueue.push([&] {
        device.destroyImageView(depthResources.imageView);
        allocator.destroyImage(depthResources.image, depthResources.memory);
    });
}

void VulkanApplication::createColorResources()
{
    DEBUG_FUNCTION
    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = swapchain.getSwapchainFormat(),
        .extent = swapchain.getSwapchainExtent3D(),
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = maxMsaaSample,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };
    vma::AllocationCreateInfo allocInfo{};
    allocInfo.usage = vma::MemoryUsage::eGpuOnly;
    std::tie(colorImage.image, colorImage.memory) = allocator.createImage(imageInfo, allocInfo);

    auto createInfo = vk_init::populateVkImageViewCreateInfo(colorImage.image, swapchain.getSwapchainFormat());
    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    colorImage.imageView = device.createImageView(createInfo);

    pivot::graphics::vk_debug::setObjectName(device, colorImage.image, "Color Image");
    pivot::graphics::vk_debug::setObjectName(device, colorImage.imageView, "Color Image view");
    swapchainDeletionQueue.push([&] {
        device.destroyImageView(colorImage.imageView);
        allocator.destroyImage(colorImage.image, colorImage.memory);
    });
}

void VulkanApplication::createImGuiDescriptorPool()
{
    DEBUG_FUNCTION;
    const vk::DescriptorPoolSize pool_sizes[]{{vk::DescriptorType::eSampler, 1000},
                                              {vk::DescriptorType::eCombinedImageSampler, 1000},
                                              {vk::DescriptorType::eSampledImage, 1000},
                                              {vk::DescriptorType::eStorageImage, 1000},
                                              {vk::DescriptorType::eUniformTexelBuffer, 1000},
                                              {vk::DescriptorType::eStorageTexelBuffer, 1000},
                                              {vk::DescriptorType::eUniformBuffer, 1000},
                                              {vk::DescriptorType::eStorageBuffer, 1000},
                                              {vk::DescriptorType::eUniformBufferDynamic, 1000},
                                              {vk::DescriptorType::eStorageBufferDynamic, 1000},
                                              {vk::DescriptorType::eInputAttachment, 1000}};

    const vk::DescriptorPoolCreateInfo pool_info{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1000,
        .poolSizeCount = std::size(pool_sizes),
        .pPoolSizes = pool_sizes,
    };

    imguiContext.pool = device.createDescriptorPool(pool_info);
    pivot::graphics::vk_debug::setObjectName(device, imguiContext.pool, "ImGui Descriptor Pool");
    mainDeletionQueue.push([&] { device.destroyDescriptorPool(imguiContext.pool); });
}

void VulkanApplication::initDearImGui()
{
    DEBUG_FUNCTION;

    ImGui_ImplVulkan_LoadFunctions(
        [](const char *function_name, void *user) {
            auto loader = reinterpret_cast<vk::DynamicLoader *>(user);
            return loader->getProcAddress<PFN_vkVoidFunction>(function_name);
        },
        &loader);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

    ImGui_ImplVulkan_InitInfo init_info{};

    init_info.Instance = instance;
    init_info.PhysicalDevice = physical_device;
    init_info.Device = device;
    init_info.QueueFamily = queueIndices.graphicsFamily.value();
    init_info.Queue = graphicsQueue;
    init_info.PipelineCache = pipelineCache;
    init_info.DescriptorPool = imguiContext.pool;
    init_info.MinImageCount = swapchain.nbOfImage();
    init_info.ImageCount = swapchain.nbOfImage();
    init_info.MSAASamples = static_cast<VkSampleCountFlagBits>(maxMsaaSample);
    init_info.CheckVkResultFn = pivot::graphics::vk_utils::vk_try;

    ImGui_ImplVulkan_Init(&init_info, renderPass);

    immediateCommand([&](vk::CommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(cmd); });
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    swapchainDeletionQueue.push([&] {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    });
}

}    // namespace pivot::graphics
#include "pivot/graphics/VulkanApplication.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/PipelineBuilders/ComputePipelineBuilder.hxx"
#include "pivot/graphics/PipelineBuilders/GraphicsPipelineBuilder.hxx"
#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <numeric>

namespace pivot::graphics
{

void VulkanApplication::createRenderPass()
{
    DEBUG_FUNCTION
    vk::AttachmentDescription colorAttachmentResolve{
        .format = swapchain.getSwapchainFormat(),
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eDontCare,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,
    };
    const auto depthFormat =
        vk_utils::findSupportedFormat(physical_device,
                                      {
                                          vk::Format::eD32Sfloat,
                                          vk::Format::eD32SfloatS8Uint,
                                          vk::Format::eD24UnormS8Uint,
                                      },
                                      vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

    renderPass.addAttachement(VulkanRenderPass::Color, swapchain.getSwapchainFormat(), maxMsaaSample,
                              vk::ImageLayout::eColorAttachmentOptimal);
    renderPass.addAttachement(VulkanRenderPass::Depth, depthFormat, maxMsaaSample,
                              vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal);
    renderPass.addAttachement(VulkanRenderPass::Resolve, colorAttachmentResolve);
    renderPass.build(device);

    swapchainDeletionQueue.push([&] { renderPass.destroy(device); });
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

    std::vector<vk::DescriptorSetLayout> setLayout = {assetStorage.getDescriptorSetLayout(),
                                                      drawResolver.getDescriptorSetLayout()};
    auto pipelineLayoutCreateInfo = vk_init::populateVkPipelineLayoutCreateInfo(setLayout, pipelinePushConstant);
    pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
    mainDeletionQueue.push([&] { device.destroy(pipelineLayout); });
}

void VulkanApplication::createCullingPipelineLayout()
{
    DEBUG_FUNCTION
    std::vector<vk::PushConstantRange> pipelinePushConstant = {vk_init::populateVkPushConstantRange(
        vk::ShaderStageFlagBits::eCompute, sizeof(gpu_object::CullingPushConstant))};
    std::vector<vk::DescriptorSetLayout> setLayout = {assetStorage.getDescriptorSetLayout(),
                                                      drawResolver.getDescriptorSetLayout()};
    auto pipelineLayoutCreateInfo = vk_init::populateVkPipelineLayoutCreateInfo(setLayout, pipelinePushConstant);
    cullingLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
    mainDeletionQueue.push([&] { device.destroy(cullingLayout); });
}

void VulkanApplication::createPipeline()
{
    DEBUG_FUNCTION

    pivot::graphics::GraphicsPipelineBuilder builder(swapchain.getSwapchainExtent());
    builder.setPipelineLayout(pipelineLayout)
        .setRenderPass(renderPass.getRenderPass())
        .setMsaaSample(maxMsaaSample)
        .setFaceCulling(vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise)
        .setVertexShaderPath("shaders/default_pbr.vert.spv")
        .setFragmentShaderPath("shaders/default_pbr.frag.spv");
    pipelineStorage.newGraphicsPipeline("pbr", builder);
    pipelineStorage.setDefault("pbr");

    builder.setVertexShaderPath("shaders/default.vert.spv").setFragmentShaderPath("shaders/default_lit.frag.spv");
    pipelineStorage.newGraphicsPipeline("lit", builder);

    builder.setFragmentShaderPath("shaders/default_unlit.frag.spv");
    pipelineStorage.newGraphicsPipeline("unlit", builder);

    builder.setPolygonMode(vk::PolygonMode::eLine);
    pipelineStorage.newGraphicsPipeline("wireframe", builder);

    builder.setPolygonMode(vk::PolygonMode::eFill)
        .setFaceCulling(vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise);
    pipelineStorage.newGraphicsPipeline("skybox", builder);

    swapchainDeletionQueue.push([&] {
        pipelineStorage.removePipeline("pbr");
        pipelineStorage.removePipeline("lit");
        pipelineStorage.removePipeline("unlit");
        pipelineStorage.removePipeline("wireframe");
        pipelineStorage.removePipeline("skybox");
    });
}

void VulkanApplication::createCullingPipeline()
{
    DEBUG_FUNCTION
    pivot::graphics::ComputePipelineBuilder builder;
    builder.setPipelineLayout(cullingLayout).setComputeShaderPath("shaders/culling.comp.spv");
    pipelineStorage.newComputePipeline("culling", builder);
}

void VulkanApplication::createFramebuffers()
{
    DEBUG_FUNCTION
    swapChainFramebuffers.resize(swapchain.nbOfImage());
    for (size_t i = 0; i < swapchain.nbOfImage(); i++) {
        std::array<vk::ImageView, 3> attachments = {colorImage.imageView, depthResources.imageView,
                                                    swapchain.getSwapchainImageView(i)};

        vk::FramebufferCreateInfo framebufferInfo{
            .renderPass = renderPass.getRenderPass(),
            .width = swapchain.getSwapchainExtent().width,
            .height = swapchain.getSwapchainExtent().height,
            .layers = 1,
        };
        framebufferInfo.setAttachments(attachments);

        swapChainFramebuffers.at(i) = device.createFramebuffer(framebufferInfo);
        vk_debug::setObjectName(device, swapChainFramebuffers.at(i), "FrameBuffer " + std::to_string(i));
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
    vk_debug::setObjectName(device, commandPool, "Main Command Pool");
    imguiContext.cmdPool = device.createCommandPool(poolInfo);
    vk_debug::setObjectName(device, imguiContext.cmdPool, "ImGui Command Pool");
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

void VulkanApplication::createDepthResources()
{
    DEBUG_FUNCTION
    vk::Format depthFormat = pivot::graphics::vk_utils::findSupportedFormat(
        physical_device, {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
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
    allocInfo.setFlags(vma::AllocationCreateFlagBits::eDedicatedMemory);
    depthResources = allocator.createImage(imageInfo, allocInfo);

    auto createInfo = vk_init::populateVkImageViewCreateInfo(depthResources.image, depthFormat);
    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    depthResources.createImageView(device, createInfo);
    transitionLayout(depthResources, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    vk_debug::setObjectName(device, depthResources.image, "Depth Image");
    vk_debug::setObjectName(device, depthResources.imageView, "Depth Image view");
    swapchainDeletionQueue.push([&] {
        device.destroyImageView(depthResources.imageView);
        allocator.destroyImage(depthResources);
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
    allocInfo.setUsage(vma::MemoryUsage::eGpuOnly);
    allocInfo.setFlags(vma::AllocationCreateFlagBits::eDedicatedMemory);

    colorImage = allocator.createImage(imageInfo, allocInfo);
    colorImage.createImageView(device);
    vk_debug::setObjectName(device, colorImage.image, "Color Image");
    vk_debug::setObjectName(device, colorImage.imageView, "Color Image view");
    swapchainDeletionQueue.push([&] {
        device.destroyImageView(colorImage.imageView);
        allocator.destroyImage(colorImage);
    });
}

void VulkanApplication::createImGuiDescriptorPool()
{
    DEBUG_FUNCTION;
    const vk::DescriptorPoolSize pool_sizes[]{
        {vk::DescriptorType::eSampler, 1000},
        {vk::DescriptorType::eCombinedImageSampler, 1000},
        {vk::DescriptorType::eSampledImage, 1000},
        {vk::DescriptorType::eStorageImage, 1000},
        {vk::DescriptorType::eUniformTexelBuffer, 1000},
        {vk::DescriptorType::eStorageTexelBuffer, 1000},
        {vk::DescriptorType::eUniformBuffer, 1000},
        {vk::DescriptorType::eStorageBuffer, 1000},
        {vk::DescriptorType::eUniformBufferDynamic, 1000},
        {vk::DescriptorType::eStorageBufferDynamic, 1000},
        {vk::DescriptorType::eInputAttachment, 1000},
    };

    const vk::DescriptorPoolCreateInfo pool_info{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1000,
        .poolSizeCount = std::size(pool_sizes),
        .pPoolSizes = pool_sizes,
    };

    imguiContext.pool = device.createDescriptorPool(pool_info);
    vk_debug::setObjectName(device, imguiContext.pool, "ImGui Descriptor Pool");
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
    init_info.DescriptorPool = imguiContext.pool;
    init_info.MinImageCount = swapchain.nbOfImage();
    init_info.ImageCount = swapchain.nbOfImage();
    init_info.MSAASamples = static_cast<VkSampleCountFlagBits>(maxMsaaSample);
    init_info.CheckVkResultFn = pivot::graphics::vk_utils::vk_try;

    ImGui_ImplVulkan_Init(&init_info, renderPass.getRenderPass());

    immediateCommand([&](vk::CommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(cmd); });
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    swapchainDeletionQueue.push([&] {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    });
}

}    // namespace pivot::graphics

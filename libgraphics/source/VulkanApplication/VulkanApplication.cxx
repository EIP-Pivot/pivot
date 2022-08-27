#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"
#include "pivot/pivot.hxx"

#ifndef PIVOT_WINDOW_ICON_PATH
const std::vector<std::string> iconFilepath;
#else
const std::vector<std::string> iconFilepath = {
    PIVOT_WINDOW_ICON_PATH "/icon_large.png",
    PIVOT_WINDOW_ICON_PATH "/icon_medium.png",
    PIVOT_WINDOW_ICON_PATH "/icon_small.png",
};
#endif

namespace pivot::graphics
{

VulkanApplication::VulkanApplication()
    : VulkanBase("Pivot Game Engine"),
      assetStorage(*this),
      pipelineStorage(*this),
      descriptorAllocator(device),
      layoutCache(device)
{
    DEBUG_FUNCTION
    if (bEnableValidationLayers && !VulkanBase::checkValidationLayerSupport(validationLayers)) {
        logger.warn("Vulkan Instance") << "Validation layers requested, but not available!";
        bEnableValidationLayers = false;
    }
    window.setIcon(iconFilepath);
    window.addKeyPressCallback(Window::Key::ESCAPE, [](Window &window, const Window::Key, const Window::Modifier) {
        window.shouldClose(true);
    });
    window.addKeyPressCallback(Window::Key::G, [this](Window &, const Window::Key, const Window::Modifier modifier) {
        if (modifier & Window::ModifierBits::Ctrl) { allocator.dumpStats(); }
    });
}

VulkanApplication::~VulkanApplication()
{
    DEBUG_FUNCTION
    if (device) {
        device.waitIdle();
    } else {
        /// if the device is not initialized, there is no need to continue further as no other ressources would have
        /// been created
        return;
    }
    if (swapchain) swapchain.destroy();
    assetStorage.destroy();

    std::ranges::for_each(graphicsRenderer, [this](auto &rendy) { rendy->onStop(*this); });
    std::ranges::for_each(computeRenderer, [this](auto &rendy) { rendy->onStop(*this); });
    std::ranges::for_each(frames, [&](Frame &fr) { fr.destroy(*this, commandPool); });

    swapchainDeletionQueue.flush();
    mainDeletionQueue.flush();
    pipelineStorage.destroy();
    descriptorAllocator.cleanup();
    layoutCache.cleanup();
    VulkanBase::destroy();
}

void VulkanApplication::init()
{
    VulkanBase::init({}, deviceExtensions, validationLayers);
    assetStorage.build(DescriptorBuilder(layoutCache, descriptorAllocator));
    initVulkanRessources();
}

void VulkanApplication::initVulkanRessources()
{
    DEBUG_FUNCTION
    if (graphicsRenderer.empty() || computeRenderer.empty()) {
        logger.err("Vulkan Application") << "No renderer found !";
        throw std::logic_error("No renderer present in the Vulkan Application.");
    }

    createCommandPool();
    std::ranges::for_each(frames, [&](Frame &fr) {
        fr.initFrame(*this, DescriptorBuilder(layoutCache, descriptorAllocator), assetStorage, commandPool);
    });

    auto size = window.getSize();
    swapchain.create(size, physical_device, device, surface);
    createDepthResources();
    createColorResources();
    createRenderPass();
    createFramebuffers();

    std::ranges::for_each(graphicsRenderer, [this](auto &rendy) {
        rendy->onInit(swapchain.getSwapchainExtent(), *this, DrawCallResolver::getDescriptorSetLayout(),
                      renderPass.getRenderPass());
    });
    std::ranges::for_each(computeRenderer,
                          [this](auto &rendy) { rendy->onInit(*this, DrawCallResolver::getDescriptorSetLayout()); });

    logger.info("Vulkan Application") << "Initialisation complete !";
}

void VulkanApplication::buildAssetStorage(AssetStorage::BuildFlags flags)
{
    device.waitIdle();

    assetStorage.build(DescriptorBuilder(layoutCache, descriptorAllocator), flags);
    std::ranges::for_each(computeRenderer, [this](auto &rendy) {
        rendy->onRecreate(swapchain.getSwapchainExtent(), *this, DrawCallResolver::getDescriptorSetLayout(),
                          renderPass.getRenderPass());
    });
    std::ranges::for_each(graphicsRenderer, [this](auto &rendy) {
        rendy->onRecreate(swapchain.getSwapchainExtent(), *this, DrawCallResolver::getDescriptorSetLayout(),
                          renderPass.getRenderPass());
    });
    logger.info("Vulkan Application") << "Asset Storage rebuild !";
}

void VulkanApplication::recreateSwapchain()
{
    DEBUG_FUNCTION

    /// do not recreate the swapchain if the window size is 0
    vk::Extent2D size = window.getSize();
    while (size.width == 0 || size.height == 0) {
        window.pollEvent();
        size = window.getSize();
    }

    logger.info("VulkanSwapchain") << "Recreating swapchain...";

    device.waitIdle();
    swapchainDeletionQueue.flush();
    swapchain.recreate(size, physical_device, device, surface);
    createColorResources();
    createDepthResources();
    createRenderPass();
    auto layout = DrawCallResolver::getDescriptorSetLayout();

    std::ranges::for_each(graphicsRenderer, [&](auto &rendy) {
        rendy->onRecreate(swapchain.getSwapchainExtent(), *this, layout, renderPass.getRenderPass());
    });

    createFramebuffers();
    logger.info("Swapchain recreation") << "New height = " << swapchain.getSwapchainExtent().height
                                        << ", New width =" << swapchain.getSwapchainExtent().width
                                        << ", numberOfImage = " << swapchain.nbOfImage();
}

VulkanApplication::DrawResult VulkanApplication::draw(DrawCallResolver::DrawSceneInformation sceneInformation,
                                                      const CameraData &cameraData,
                                                      std::optional<vk::Rect2D> renderArea)
try {
    if (!pivot_check(!graphicsRenderer.empty() && !computeRenderer.empty(), "No Renderers are setup")) {
        // This is technically a success, even thought nothing was drawn.
        return DrawResult::Success;
    }
    auto &frame = frames.get();
    vk_utils::vk_try(device.waitForFences(frame.inFlightFences, VK_TRUE, UINT64_MAX));

    const uint32_t imageIndex = swapchain.getNextImageIndex(UINT64_MAX, frame.imageAvailableSemaphore);

    auto &cmd = frame.cmdBuffer;

    device.resetFences(frame.inFlightFences);
    cmd.reset();

    std::vector<vk::CommandBuffer> computeBuffer;

    frame.drawResolver.prepareForDraw(sceneInformation);

    const std::array<float, 4> vClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    std::array<vk::ClearValue, 2> clearValues{};
    clearValues.at(0).color = vk::ClearColorValue{vClearColor};
    clearValues.at(1).depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    RenderingContext context{
        .renderArea = renderArea.value_or(vk::Rect2D{
            .offset = {0, 0},
            .extent = swapchain.getSwapchainExtent(),
        }),
        .viewport = swapchain.getSwapchainExtent(),
    };
    context.renderArea.extent.height = std::max(context.renderArea.extent.height, 1u);
    context.renderArea.extent.width = std::max(context.renderArea.extent.width, 1u);
    context.renderArea.offset.x = std::max(context.renderArea.offset.x, 0);
    context.renderArea.offset.y = std::max(context.renderArea.offset.y, 0);

    vk::RenderPassBeginInfo renderPassInfo{
        .renderPass = renderPass.getRenderPass(),
        .framebuffer = swapChainFramebuffers.at(imageIndex),
        .renderArea =
            {
                .offset = {0, 0},
                .extent = swapchain.getSwapchainExtent(),
            },
        .clearValueCount = clearValues.size(),
        .pClearValues = clearValues.data(),
    };
    vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,

    };
    vk_utils::vk_try(cmd.begin(&beginInfo));
    vk_debug::beginRegion(cmd, "main command", {1.f, 1.f, 1.f, 1.f});
    for (auto &rendy: computeRenderer) rendy->onDraw(context, cameraData, frame.drawResolver, cmd);
    cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    for (auto &rendy: graphicsRenderer) rendy->onDraw(context, cameraData, frame.drawResolver, cmd);
    cmd.endRenderPass();
    vk_debug::endRegion(cmd);
    cmd.end();

    std::array<vk::Semaphore, 1> waitSemaphores{
        frame.imageAvailableSemaphore,
    };
    std::array<vk::PipelineStageFlags, 1> waitStages{
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
    };
    std::array<vk::Semaphore, 1> signalSemaphores{
        frame.renderFinishedSemaphore,
    };
    std::array<vk::CommandBuffer, 1> submitCmd{
        cmd,
    };
    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = waitSemaphores.size(),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStages.data(),
        .signalSemaphoreCount = signalSemaphores.size(),
        .pSignalSemaphores = signalSemaphores.data(),
    };
    submitInfo.setCommandBuffers(submitCmd);
    graphicsQueue.submit(submitInfo, frame.inFlightFences);

    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = signalSemaphores.size(),
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &(swapchain.getSwapchain()),
        .pImageIndices = &imageIndex,
    };
    vk_utils::vk_try(presentQueue.presentKHR(presentInfo));
    frames.advance();
    return DrawResult::Success;
} catch (const vk::OutOfDateKHRError &e) {
    logger.debug("VulkanApplication/OutOfDateKHRError") << e.what();
    recreateSwapchain();
    return DrawResult::FrameSkipped;
} catch (const vk::Error &e) {
    logger.debug("VulkanApplication/Vulkan Error") << e.what();
    return DrawResult::Error;
}

}    // namespace pivot::graphics

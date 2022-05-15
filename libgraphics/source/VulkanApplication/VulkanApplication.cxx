#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>

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
    window.setKeyPressCallback(Window::Key::ESCAPE,
                               [](Window &window, const Window::Key) { window.shouldClose(true); });
}

VulkanApplication::~VulkanApplication()
{
    DEBUG_FUNCTION
    if (device)
        device.waitIdle();
    else {
        /// if the device is not initialized, there is no need to continue further as no other ressources would have
        /// been created
        return;
    }
    if (swapchain) swapchain.destroy();
    assetStorage.destroy();

    std::ranges::for_each(graphicsRenderer, [this](auto &pair) { pair.first->onStop(*this); });
    std::ranges::for_each(computeRenderer, [this](auto &pair) { pair.first->onStop(*this); });
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

    std::ranges::for_each(graphicsRenderer, [this](auto &pair) {
        pair.first->onInit(swapchain.getSwapchainExtent(), *this, frames[0].drawResolver.getDescriptorSetLayout(),
                           renderPass.getRenderPass());
    });
    std::ranges::for_each(computeRenderer, [this](auto &pair) {
        pair.first->onInit(*this, frames[0].drawResolver.getDescriptorSetLayout());
    });

    createCommandBuffers();
    postInitialization();
    logger.info("Vulkan Application") << "Initialisation complete !";
}

void VulkanApplication::postInitialization() {}

void VulkanApplication::buildAssetStorage(AssetStorage::BuildFlags flags)
{
    device.waitIdle();

    assetStorage.build(DescriptorBuilder(layoutCache, descriptorAllocator), flags);
    std::ranges::for_each(computeRenderer, [this](auto &pair) {
        pair.first->onRecreate(swapchain.getSwapchainExtent(), *this, frames[0].drawResolver.getDescriptorSetLayout(),
                               renderPass.getRenderPass());
    });
    std::ranges::for_each(graphicsRenderer, [this](auto &pair) {
        pair.first->onRecreate(swapchain.getSwapchainExtent(), *this, frames[0].drawResolver.getDescriptorSetLayout(),
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
    createCommandBuffers();
    auto layout = frames[0].drawResolver.getDescriptorSetLayout();

    std::ranges::for_each(graphicsRenderer, [&](std::pair<std::unique_ptr<IGraphicsRenderer>, CommandVector> &pair) {
        pair.first->onRecreate(swapchain.getSwapchainExtent(), *this, layout, renderPass.getRenderPass());
    });

    createFramebuffers();
    logger.info("Swapchain recreation") << "New height = " << swapchain.getSwapchainExtent().height
                                        << ", New width =" << swapchain.getSwapchainExtent().width
                                        << ", numberOfImage = " << swapchain.nbOfImage();

    postInitialization();
}

void VulkanApplication::draw(DrawCallResolver::DrawSceneInformation sceneInformation, const CameraData &cameraData)
try {
    assert(!graphicsRenderer.empty() && !computeRenderer.empty());
    assert(currentFrame < MaxFrameInFlight);
    auto &frame = frames[currentFrame];
    vk_utils::vk_try(device.waitForFences(frame.inFlightFences, VK_TRUE, UINT64_MAX));

    uint32_t imageIndex = swapchain.getNextImageIndex(UINT64_MAX, frame.imageAvailableSemaphore);

    auto &cmd = frame.cmdBuffer;

    device.resetFences(frame.inFlightFences);
    cmd.reset();

    std::vector<vk::CommandBuffer> secondaryBuffer;
    std::vector<vk::CommandBuffer> computeBuffer;
    vk::CommandBufferInheritanceInfo inheritanceInfo{
        .renderPass = renderPass.getRenderPass(),
        .subpass = 0,
        .framebuffer = swapChainFramebuffers.at(imageIndex),
    };
    vk::CommandBufferBeginInfo drawBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit | vk::CommandBufferUsageFlagBits::eRenderPassContinue,
        .pInheritanceInfo = &inheritanceInfo,
    };
    vk::CommandBufferBeginInfo computeInfo{
        .pInheritanceInfo = &inheritanceInfo,
    };

    frame.drawResolver.prepareForDraw(sceneInformation);
    for (auto &[rendy, buffers]: computeRenderer) {
        auto &cmdBuf = buffers.at(currentFrame);
        cmdBuf.reset();
        vk_utils::vk_try(cmdBuf.begin(&computeInfo));
        rendy->onDraw(cameraData, frame.drawResolver, cmdBuf);
        cmdBuf.end();
        computeBuffer.push_back(cmdBuf);
    }
    for (auto &[rendy, buffers]: graphicsRenderer) {
        auto &cmdBuf = buffers.at(currentFrame);
        cmdBuf.reset();
        vk_utils::vk_try(cmdBuf.begin(&drawBeginInfo));
        rendy->onDraw(cameraData, frame.drawResolver, cmdBuf);
        cmdBuf.end();
        secondaryBuffer.push_back(cmdBuf);
    }

    const std::array<float, 4> vClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    std::array<vk::ClearValue, 2> clearValues{};
    clearValues.at(0).color = vk::ClearColorValue{vClearColor};
    clearValues.at(1).depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
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
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit | vk::CommandBufferUsageFlagBits::eRenderPassContinue,

    };
    vk_utils::vk_try(cmd.begin(&beginInfo));
    vk_debug::beginRegion(cmd, "main command", {1.f, 1.f, 1.f, 1.f});
    cmd.executeCommands(computeBuffer);
    cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
    cmd.executeCommands(secondaryBuffer);
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
    assert(signalSemaphores.size() == waitStages.size());
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
    currentFrame = (currentFrame + 1) % MaxFrameInFlight;
} catch (const vk::OutOfDateKHRError &) {
    return recreateSwapchain();
}

}    // namespace pivot::graphics

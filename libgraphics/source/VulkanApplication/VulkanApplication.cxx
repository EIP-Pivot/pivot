#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>

namespace pivot::graphics
{

VulkanApplication::VulkanApplication()
    : VulkanBase("Pivot Game Engine", true), assetStorage(*this), pipelineStorage(*this)
{
    DEBUG_FUNCTION;

#if defined(CULLING_DEBUG)
    logger.warn("Culling") << "Culling camera are enabled !";
#endif

    if (bEnableValidationLayers && !VulkanBase::checkValidationLayerSupport(validationLayers)) {
        logger.warn("Vulkan Instance") << "Validation layers requested, but not available!";

        bEnableValidationLayers = false;
    }
    window.setKeyPressCallback(Window::Key::ESCAPE,
                               [](Window &window, const Window::Key) { window.shouldClose(true); });
}

VulkanApplication::~VulkanApplication()
{
    DEBUG_FUNCTION
    if (device) device.waitIdle();
    if (swapchain) swapchain.destroy();
    assetStorage.destroy();

    std::for_each(graphicsRenderer.begin(), graphicsRenderer.end(), [this](auto &pair) { pair.first->onStop(*this); });
    std::for_each(computeRenderer.begin(), computeRenderer.end(), [this](auto &pair) { pair.first->onStop(*this); });
    std::for_each(frames.begin(), frames.end(), [&](Frame &fr) { fr.destroy(*this, commandPool); });

    swapchainDeletionQueue.flush();
    mainDeletionQueue.flush();
    pipelineStorage.destroy();
    VulkanBase::destroy();
}

void VulkanApplication::init()
{
    VulkanBase::init({}, deviceExtensions, validationLayers);
    assetStorage.build();
    initVulkanRessources();
}

void VulkanApplication::initVulkanRessources()
{
    DEBUG_FUNCTION

    createCommandPool();
    std::for_each(frames.begin(), frames.end(), [&](Frame &fr) { fr.initFrame(*this, assetStorage, commandPool); });
    auto size = window.getSize();
    swapchain.create(size, physical_device, device, surface);
    createDepthResources();
    createColorResources();
    createRenderPass();
    createFramebuffers();

    auto layout = frames[0].drawResolver.getDescriptorSetLayout();
    for (auto &[rendy, buffers]: graphicsRenderer)
        rendy->onInit(swapchain.getSwapchainExtent(), *this, layout, renderPass.getRenderPass());
    for (auto &[rendy, buffers]: computeRenderer) rendy->onInit(*this, layout);

    createCommandBuffers();
    postInitialization();
    logger.info("Vulkan Application") << "Initialisation complete !";
}

void VulkanApplication::postInitialization() {}

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

    std::for_each(graphicsRenderer.begin(), graphicsRenderer.end(),
                  [&](std::pair<std::unique_ptr<IGraphicsRenderer>, CommandVector> &pair) {
                      pair.first->onRecreate(swapchain.getSwapchainExtent(), *this, layout, renderPass.getRenderPass());
                  });

    createFramebuffers();
    logger.info("Swapchain recreation") << "New height = " << swapchain.getSwapchainExtent().height
                                        << ", New width =" << swapchain.getSwapchainExtent().width
                                        << ", numberOfImage = " << swapchain.nbOfImage();

    postInitialization();
}

void VulkanApplication::draw(std::vector<std::reference_wrapper<const RenderObject>> &sceneInformation,
                             const CameraData &cameraData
#ifdef CULLING_DEBUG
                             ,
                             const std::optional<std::reference_wrapper<const CameraData>> cullingCameraData
#endif
)
try {
    auto &frame = frames[currentFrame];
    vk_utils::vk_try(device.waitForFences(frame.inFlightFences, VK_TRUE, UINT64_MAX));

    uint32_t imageIndex = swapchain.getNextImageIndex(UINT64_MAX, frame.imageAvailableSemaphore);

    auto &cmd = frame.cmdBuffer;

    device.resetFences(frame.inFlightFences);
    cmd.reset();

    vk::Semaphore waitSemaphores[] = {frame.imageAvailableSemaphore};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore signalSemaphores[] = {frame.renderFinishedSemaphore};

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
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
        .pClearValues = clearValues.data(),
    };

    frame.drawResolver.prepareForDraw(sceneInformation);

    vk::CommandBufferInheritanceInfo inheritanceInfo{
        .renderPass = renderPass.getRenderPass(),
        .subpass = 0,
        .framebuffer = swapChainFramebuffers.at(imageIndex),
    };
    vk::CommandBufferBeginInfo drawBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue,
        .pInheritanceInfo = &inheritanceInfo,
    };
    vk::CommandBufferBeginInfo computeInfo{
        .pInheritanceInfo = &inheritanceInfo,
    };

    std::vector<vk::CommandBuffer> secondaryBuffer;
    std::vector<vk::CommandBuffer> computeBuffer;
    for (auto &[rendy, buffers]: computeRenderer) {
        auto &cmdBuf = buffers.at(imageIndex);
        cmdBuf.reset();
        vk_utils::vk_try(cmdBuf.begin(&computeInfo));
        rendy->onDraw(cameraData, frame.drawResolver, cmdBuf);
        cmdBuf.end();
        computeBuffer.push_back(cmdBuf);
    }
    for (auto &[rendy, buffers]: graphicsRenderer) {
        auto &cmdBuf = buffers.at(imageIndex);
        cmdBuf.reset();
        vk_utils::vk_try(cmdBuf.begin(&drawBeginInfo));
        rendy->onDraw(cameraData, frame.drawResolver, cmdBuf);
        cmdBuf.end();
        secondaryBuffer.push_back(cmdBuf);
    }

    // #ifdef CULLING_DEBUG
    //     auto cullingCameraDataSelected = cullingCameraData.value_or(std::ref(cameraData)).get();
    // #else
    //     auto cullingCameraDataSelected = cameraData;
    // #endif

    vk::CommandBufferBeginInfo beginInfo;
    vk_utils::vk_try(cmd.begin(&beginInfo));
    vk_debug::beginRegion(cmd, "main command", {1.f, 1.f, 1.f, 1.f});
    cmd.executeCommands(computeBuffer);
    cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
    cmd.executeCommands(secondaryBuffer);
    cmd.endRenderPass();
    vk_debug::endRegion(cmd);
    cmd.end();

    const std::array<vk::CommandBuffer, 1> submitCmd{cmd};
    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };
    submitInfo.setCommandBuffers(submitCmd);
    graphicsQueue.submit(submitInfo, frame.inFlightFences);

    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
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

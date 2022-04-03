#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>

namespace pivot::graphics
{

VulkanApplication::VulkanApplication()
    : VulkanBase("Pivot Game Engine", true),
      assetStorage(*this),
      pipelineStorage(*this),
      culling(pipelineStorage, assetStorage),
      graphics(pipelineStorage, assetStorage),
      ImGui(pipelineStorage, assetStorage)
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
    ImGui.onStop(*this);
    graphics.onStop(*this);
    culling.onStop(*this);
    assetStorage.destroy();
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
    createCommandBuffers();
    createDepthResources();
    createColorResources();
    createRenderPass();

    auto layout = frames[0].drawResolver.getDescriptorSetLayout();
    culling.onInit(*this, layout);
    graphics.onInit(swapchain.getSwapchainExtent(), *this, layout, renderPass.getRenderPass());
    ImGui.onInit(swapchain.getSwapchainExtent(), *this, layout, renderPass.getRenderPass());
    createFramebuffers();

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
    createCommandBuffers();
    createColorResources();
    createDepthResources();
    createRenderPass();
    auto layout = frames[0].drawResolver.getDescriptorSetLayout();
    graphics.onRecreate(swapchain.getSwapchainExtent(), *this, layout, renderPass.getRenderPass());
    ImGui.onRecreate(swapchain.getSwapchainExtent(), *this, layout, renderPass.getRenderPass());

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
    auto &cullingBuff = cullingBuffer.at(imageIndex);
    auto &imGui = ImGuiBuffer.at(imageIndex);
    auto &graphicsBuff = graphicsBuffer.at(imageIndex);

    device.resetFences(frame.inFlightFences);
    cmd.reset();
    cullingBuff.reset();
    imGui.reset();
    graphicsBuff.reset();

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

    vk_utils::vk_try(cullingBuff.begin(&computeInfo));
    {
        culling.onDraw(cameraData, frame.drawResolver, cullingBuff);
    }
    cullingBuff.end();

    vk_utils::vk_try(imGui.begin(&drawBeginInfo));
    {
        ImGui.onDraw(cameraData, frame.drawResolver, imGui);
    }
    imGui.end();

    vk_utils::vk_try(graphicsBuff.begin(&drawBeginInfo));
    {
        graphics.onDraw(cameraData, frame.drawResolver, graphicsBuff);
    }
    graphicsBuff.end();

#ifdef CULLING_DEBUG
    auto cullingCameraDataSelected = cullingCameraData.value_or(std::ref(cameraData)).get();
#else
    auto cullingCameraDataSelected = cameraData;
#endif

    std::array<vk::CommandBuffer, 2> secondaryBuffer{graphicsBuff, imGui};
    std::array<vk::CommandBuffer, 1> computeBuffer{cullingBuff};
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

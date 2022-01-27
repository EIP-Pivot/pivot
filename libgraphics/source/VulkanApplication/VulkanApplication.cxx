#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/culling.hxx"
#include "pivot/graphics/math.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <algorithm>

namespace pivot::graphics
{

VulkanApplication::VulkanApplication()
    : VulkanBase("Pivot Game Engine", true), assetStorage(*this), drawResolver(*this, assetStorage)
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
    swapchainDeletionQueue.flush();
    mainDeletionQueue.flush();
    drawResolver.destroy();
    VulkanBase::destroy();
}

void VulkanApplication::init()
{
    VulkanBase::init({}, deviceExtensions, validationLayers);
    drawResolver.init();
    assetStorage.build();
    initVulkanRessources();
}

void VulkanApplication::initVulkanRessources()
{
    DEBUG_FUNCTION

    createPipelineCache();
    createSyncStructure();
    createRessourcesDescriptorSetLayout();
    createPipelineLayout();
    createCullingPipelineLayout();
    createCommandPool();
    createDescriptorPool();
    createImGuiDescriptorPool();
    createCullingPipeline();

    auto size = window.getSize();
    swapchain.create(size, physical_device, device, surface);

    createRenderPass();
    createPipeline();
    createDepthResources();
    createColorResources();
    createFramebuffers();

    createTextureSampler();
    createRessourcesDescriptorSets();
    createCommandBuffers();
    initDearImGui();

    postInitialization();
}

void VulkanApplication::postInitialization() { DEBUG_FUNCTION }

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
    createRenderPass();
    createPipeline();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createCommandBuffers();
    initDearImGui();
    logger.info("Swapchain recreation") << "New height = " << swapchain.getSwapchainExtent().height
                                        << ", New width =" << swapchain.getSwapchainExtent().width
                                        << ", numberOfImage = " << swapchain.nbOfImage();

    postInitialization();
}

void VulkanApplication::draw(const std::vector<std::reference_wrapper<const RenderObject>> &sceneInformation,
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
    allocator.setCurrentFrameIndex(imageIndex);

    auto &cmd = commandBuffersPrimary[imageIndex];
    auto &drawCmd = commandBuffersSecondary[imageIndex];
    auto &imguiCmd = imguiContext.cmdBuffer[imageIndex];

    device.resetFences(frame.inFlightFences);
    cmd.reset();
    drawCmd.reset();
    imguiCmd.reset();

    vk::Semaphore waitSemaphores[] = {frame.imageAvailableSemaphore};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore signalSemaphores[] = {frame.renderFinishedSemaphore};

    const std::array<float, 4> vClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    std::array<vk::ClearValue, 2> clearValues{};
    clearValues.at(0).color = vk::ClearColorValue{vClearColor};
    clearValues.at(1).depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    vk::RenderPassBeginInfo renderPassInfo{
        .renderPass = renderPass,
        .framebuffer = swapChainFramebuffers[imageIndex],
        .renderArea =
            {
                .offset = {0, 0},
                .extent = swapchain.getSwapchainExtent(),
            },
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
        .pClearValues = clearValues.data(),
    };

    const gpuObject::VertexPushConstant vertexCamere{
        .viewProjection = cameraData.viewProjection,
    };
    const gpuObject::FragmentPushConstant fragmentCamera{
        .position = cameraData.position,
    };

#ifdef CULLING_DEBUG
    auto cullingCameraDataSelected = cullingCameraData.value_or(std::ref(cameraData)).get();
#else
    auto cullingCameraDataSelected = cameraData;
#endif

    drawResolver.prepareForDraw(sceneInformation, cullingCameraDataSelected, currentFrame);

    const gpuObject::CullingPushConstant cullingCamera{
        .viewProjection = cullingCameraDataSelected.viewProjection,
        .drawCount = static_cast<uint32_t>(drawResolver.getFrameData(currentFrame).packedDraws.size()),
    };

    vk::CommandBufferInheritanceInfo inheritanceInfo{
        .renderPass = renderPass,
        .subpass = 0,
        .framebuffer = swapChainFramebuffers.at(imageIndex),
    };

    // ImGui draw
    {
        vk::CommandBufferBeginInfo imguiBeginInfo{
            .flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue,
            .pInheritanceInfo = &inheritanceInfo,
        };
        vk_utils::vk_try(imguiCmd.begin(&imguiBeginInfo));
        pivot::graphics::vk_debug::beginRegion(imguiCmd, "Imgui Commands", {1.f, 0.f, 0.f, 1.f});
        if (auto imguiData = ImGui::GetDrawData(); imguiData != nullptr) {
            ImGui_ImplVulkan_RenderDrawData(imguiData, imguiCmd);
        }
        pivot::graphics::vk_debug::endRegion(imguiCmd);
        imguiCmd.end();
    }

    // Normal draw
    {

        vk::DeviceSize offset = 0;
        vk::CommandBufferBeginInfo drawBeginInfo{
            .flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue,
            .pInheritanceInfo = &inheritanceInfo,
        };
        vk_utils::vk_try(drawCmd.begin(&drawBeginInfo));
        pivot::graphics::vk_debug::beginRegion(imguiCmd, "Draw Commands", {0.f, 1.f, 0.f, 1.f});
        if (drawResolver.getFrameData(currentFrame).packedDraws.size() > 0) {
            drawCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
            drawCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0,
                                       drawResolver.getFrameData(currentFrame).objectDescriptor, nullptr);
            drawCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, ressourceDescriptorSet,
                                       nullptr);
            drawCmd.pushConstants<gpuObject::VertexPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0,
                                                                 vertexCamere);
            drawCmd.pushConstants<gpuObject::FragmentPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eFragment,
                                                                   sizeof(gpuObject::VertexPushConstant),
                                                                   fragmentCamera);
            drawCmd.bindVertexBuffers(0, assetStorage.getVertexBuffer().buffer, offset);
            drawCmd.bindIndexBuffer(assetStorage.getIndexBuffer().buffer, 0, vk::IndexType::eUint32);

            if (deviceFeature.multiDrawIndirect == VK_TRUE) {
                drawCmd.drawIndexedIndirect(drawResolver.getFrameData(currentFrame).indirectBuffer.buffer, 0,
                                            drawResolver.getFrameData(currentFrame).packedDraws.size(),
                                            sizeof(vk::DrawIndexedIndirectCommand));
            } else {
                for (const auto &draw: drawResolver.getFrameData(currentFrame).packedDraws) {
                    drawCmd.drawIndexedIndirect(drawResolver.getFrameData(currentFrame).indirectBuffer.buffer,
                                                draw.first * sizeof(vk::DrawIndexedIndirectCommand), draw.count,
                                                sizeof(vk::DrawIndexedIndirectCommand));
                }
            }
        }
        pivot::graphics::vk_debug::endRegion(drawCmd);
        drawCmd.end();
    }

    std::array<vk::CommandBuffer, 2> secondaryBuffer{drawCmd, imguiCmd};
    vk::CommandBufferBeginInfo beginInfo;
    vk_utils::vk_try(cmd.begin(&beginInfo));
    pivot::graphics::vk_debug::beginRegion(imguiCmd, "main command", {1.f, 1.f, 1.f, 1.f});

    {
        pivot::graphics::vk_debug::beginRegion(cmd, "culling pass", {1.f, 0.f, 1.f, 1.f});
        vk::BufferMemoryBarrier barrier{
            .srcAccessMask = vk::AccessFlagBits::eIndirectCommandRead,
            .dstAccessMask = vk::AccessFlagBits::eShaderWrite,
            .srcQueueFamilyIndex = queueIndices.graphicsFamily.value(),
            .dstQueueFamilyIndex = queueIndices.graphicsFamily.value(),
            .buffer = drawResolver.getFrameData(currentFrame).indirectBuffer.buffer,
            .size = drawResolver.getFrameData(currentFrame).indirectBuffer.size,
        };
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, cullingLayout, 0,
                               drawResolver.getFrameData(currentFrame).objectDescriptor, nullptr);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, cullingLayout, 1, ressourceDescriptorSet, nullptr);
        cmd.pushConstants<gpuObject::CullingPushConstant>(cullingLayout, vk::ShaderStageFlagBits::eCompute, 0,
                                                          cullingCamera);
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, cullingPipeline);
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eDrawIndirect, vk::PipelineStageFlagBits::eComputeShader, {}, {},
                            barrier, {});
        cmd.dispatch((drawResolver.getFrameData(currentFrame).packedDraws.size() / 256) + 1, 1, 1);

        barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eIndirectCommandRead;
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eDrawIndirect, {}, {},
                            barrier, {});
        pivot::graphics::vk_debug::endRegion(cmd);
    }

    cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
    cmd.executeCommands(secondaryBuffer);
    cmd.endRenderPass();
    pivot::graphics::vk_debug::endRegion(cmd);
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
    pivot::graphics::vk_utils::vk_try(presentQueue.presentKHR(presentInfo));
    currentFrame = (currentFrame + 1) % MaxFrameInFlight;
} catch (const vk::OutOfDateKHRError &) {
    return recreateSwapchain();
}

}    // namespace pivot::graphics
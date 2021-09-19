#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_utils.hxx"
#include <Logger.hpp>
#include <algorithm>

VulkanApplication::VulkanApplication(): VulkanLoader(), window("Vulkan", 800, 600)
{
    DEBUG_FUNCTION window.setKeyEventCallback(
        Window::Key::ESCAPE,
        [](Window &window, const Window::Key key, const Window::KeyAction action) { window.shouldClose(true); });
}

VulkanApplication::~VulkanApplication()
{
    DEBUG_FUNCTION
    if (device) device.waitIdle();
    if (swapchain) swapchain.destroy();
    swapchainDeletionQueue.flush();
    mainDeletionQueue.flush();
}

void VulkanApplication::init() { initVulkanRessources(); }

void VulkanApplication::draw(const I3DScene &scene, const Camera &camera, float fElapsedTime)
try {
    auto &frame = frames[currentFrame];
    uint32_t imageIndex;
    vk::Result result;

    VK_TRY(device.waitForFences(frame.inFlightFences, VK_TRUE, UINT64_MAX));
    std::tie(result, imageIndex) =
        device.acquireNextImageKHR(swapchain.getSwapchain(), UINT64_MAX, frame.imageAvailableSemaphore);

    vk_utils::vk_try(result);
    auto &cmd = commandBuffers[imageIndex];

    device.resetFences(frame.inFlightFences);
    cmd.reset();

    vk::Semaphore waitSemaphores[] = {frame.imageAvailableSemaphore};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore signalSemaphores[] = {frame.renderFinishedSemaphore};

    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };

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
    auto gpuCamera = camera.getGPUCameraData(80.0f, swapchain.getAspectRatio(), 0.1f);
    auto sceneInformation = scene.getSceneInformations();
    auto drawBatch = buildDrawBatch(sceneInformation);
    buildIndirectBuffers(drawBatch, frame);

    std::vector<gpuObject::UniformBufferObject> sceneData;
    std::transform(sceneInformation.begin(), sceneInformation.end(), std::back_inserter(sceneData),
                   [this](const auto &i) {
                       return gpuObject::UniformBufferObject(i.objectInformation, loadedTextures, materials);
                   });
    copyBuffer(frame.data.uniformBuffers, sceneData);

    vk::DeviceSize offset = 0;
    vk::CommandBufferBeginInfo beginInfo;
    VK_TRY(cmd.begin(&beginInfo));
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, frame.data.objectDescriptor, nullptr);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, texturesSet, nullptr);
    cmd.pushConstants<Camera::GPUCameraData>(
        pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, gpuCamera);
    cmd.bindVertexBuffers(0, vertexBuffers.buffer, offset);
    cmd.bindIndexBuffer(indicesBuffers.buffer, 0, vk::IndexType::eUint32);
    cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    {
        for (const auto &draw: drawBatch) {
            cmd.drawIndexedIndirect(frame.indirectBuffer.buffer, draw.first * sizeof(vk::DrawIndexedIndirectCommand),
                                    draw.count, sizeof(vk::DrawIndexedIndirectCommand));
        }
    }
    cmd.endRenderPass();
    cmd.end();
    graphicsQueue.submit(submitInfo, frame.inFlightFences);

    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = &(swapchain.getSwapchain()),
        .pImageIndices = &imageIndex,
    };
    vk_utils::vk_try(presentQueue.presentKHR(presentInfo));
    currentFrame = (currentFrame + 1) % MAX_FRAME_FRAME_IN_FLIGHT;
} catch (const vk::OutOfDateKHRError &se) {
    return recreateSwapchain();
}

std::vector<VulkanApplication::DrawBatch> VulkanApplication::buildDrawBatch(std::vector<RenderObject> &object)
{
    // std::sort(object.begin(), object.end(),
    //           [](const auto &first, const auto &second) { return first.meshID == second.meshID; });
    if (object.empty()) return {};
    if (object.size() >= MAX_OBJECT) throw TooManyObjectInSceneError();

    std::vector<DrawBatch> packedDraws;
    packedDraws.push_back({
        .meshId = object.at(0).meshID,
        .first = 0,
        .count = 1,
    });

    for (uint32_t i = 1; i < object.size(); i++) {
        // if (object[i].meshID == packedDraws.back().meshId) {
        //     packedDraws.back().count++;
        // } else {
        packedDraws.push_back({
            .meshId = object.at(i).meshID,
            .first = i,
            .count = 1,
        });
        //}
    }
    return packedDraws;
}

void VulkanApplication::buildIndirectBuffers(const std::vector<DrawBatch> &packedDraws, Frame &frame)
{
    void *sceneData = nullptr;
    vmaMapMemory(allocator, frame.indirectBuffer.memory, &sceneData);
    auto *buffer = (VkDrawIndexedIndirectCommand *)sceneData;

    for (uint32_t i = 0; i < packedDraws.size(); i++) {
        const auto &mesh = loadedMeshes.at(packedDraws[i].meshId);

        buffer[i].firstIndex = mesh.indicesOffset;
        buffer[i].indexCount = mesh.indicesSize;
        buffer[i].vertexOffset = mesh.verticiesOffset;
        buffer[i].instanceCount = 1;
        buffer[i].firstInstance = i;
    }
    vmaUnmapMemory(allocator, frame.indirectBuffer.memory);
}

void VulkanApplication::initVulkanRessources()
{
    DEBUG_FUNCTION
    createInstance();
    createDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createAllocator();
    createSyncStructure();
    createIndirectBuffer();
    createDescriptorSetLayout();
    createTextureDescriptorSetLayout();
    createPipelineCache();
    createPipelineLayout();
    createCommandPool();
    createDescriptorPool();

    swapchain.init(window, physical_device, device, surface);

    createUniformBuffers();
    createRenderPass();
    createPipeline();
    createDepthResources();
    createColorResources();
    createFramebuffers();
    createDescriptorSets();

    this->pushModelsToGPU();
    this->pushTexturesToGPU();

    createTextureSampler();
    createTextureDescriptorSets();
    createCommandBuffers();

    materials["white"] = {
        .ambientColor = {1.0f, 1.0f, 1.0f, 1.0f},
        .diffuse = {1.0f, 1.0f, 1.0f, 1.0f},
        .specular = {1.0f, 1.0f, 1.0f, 1.0f},
    };
    postInitialization();
}

void VulkanApplication::postInitialization()
{
    DEBUG_FUNCTION

    std::vector<gpuObject::Material> materialStor;
    std::transform(materials.begin(), materials.end(), std::back_inserter(materialStor),
                   [this](const auto &i) { return i.second; });

    for (auto &frame: frames) { copyBuffer(frame.data.materialBuffer, materialStor); }
}

void VulkanApplication::recreateSwapchain()
{
    DEBUG_FUNCTION
    // int width = 0, height = 0;
    // glfwGetFramebufferSize(window.getWindow(), &width, &height);
    // while (width == 0 || height == 0) {
    //     glfwGetFramebufferSize(window.getWindow(), &width, &height);
    //     glfwWaitEvents();
    // }

    logger->info("Swapchain") << "Recreaing swapchain...";
    LOGGER_ENDL;

    device.waitIdle();
    swapchainDeletionQueue.flush();

    swapchain.recreate(window, physical_device, device, surface);
    createRenderPass();
    createPipeline();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createCommandBuffers();
    logger->info("Swapchain") << "Swapchain recreation complete... { height=" << swapchain.getSwapchainExtent().height
                              << ", width =" << swapchain.getSwapchainExtent().width
                              << ", numberOfImage = " << swapchain.nbOfImage() << " }";
    LOGGER_ENDL;
    postInitialization();
}
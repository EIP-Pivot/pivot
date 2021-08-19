#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>
#include <algorithm>

VulkanApplication::VulkanApplication() {}

VulkanApplication::~VulkanApplication()
{
    if (device != VK_NULL_HANDLE) vkDeviceWaitIdle(device);
    swapchain.destroy();
    swapchainDeletionQueue.flush();
    mainDeletionQueue.flush();
}

void VulkanApplication::init(IWindow &win)
{
    window = win;
    initVulkanRessources();
}

void VulkanApplication::draw(const I3DScene &scene, const Camera &camera, float fElapsedTime)
{
    auto &frame = frames[currentFrame];
    uint32_t imageIndex;

    VK_TRY(vkWaitForFences(device, 1, &frame.inFlightFences, VK_TRUE, UINT64_MAX));

    auto result = vkAcquireNextImageKHR(device, swapchain.getSwapchain(), UINT64_MAX, frame.imageAvailableSemaphore,
                                        nullptr, &imageIndex);
    if (vk_utils::isSwapchainInvalid(result, VK_ERROR_OUT_OF_DATE_KHR)) { return recreateSwapchain(); }

    auto &cmd = commandBuffers[imageIndex];

    VK_TRY(vkResetFences(device, 1, &frame.inFlightFences));
    VK_TRY(vkResetCommandBuffer(cmd, 0));
    VkSemaphore waitSemaphores[] = {frame.imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {frame.renderFinishedSemaphore};

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };

    std::array<VkClearValue, 2> clearValues{};
    clearValues.at(0).color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    clearValues.at(1).depthStencil = {1.0f, 0};

    VkDeviceSize offsets = 0;
    VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
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
    auto gpuCamera = camera.getGPUCameraData(80, swapchain.getAspectRatio());

    auto sceneInformation = scene.getSceneInformations();
    auto drawBatch = buildDrawBatch(sceneInformation);
    buildIndirectBuffers(drawBatch, frame);

    std::vector<gpuObject::UniformBufferObject> sceneData;
    std::transform(sceneInformation.begin(), sceneInformation.end(), std::back_inserter(sceneData),
                   [](const auto &i) { return gpuObject::UniformBufferObject(i.objectInformation); });

    void *objectData = nullptr;
    vmaMapMemory(allocator, frame.data.uniformBuffers.memory, &objectData);
    auto *objectSSBI = (gpuObject::UniformBufferObject *)objectData;
    for (unsigned i = 0; i < sceneData.size(); i++) { objectSSBI[i] = sceneData.at(i); }
    vmaUnmapMemory(allocator, frame.data.uniformBuffers.memory);

    VK_TRY(vkBeginCommandBuffer(cmd, &beginInfo));
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frame.data.objectDescriptor, 0,
                            nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &texturesSet, 0, nullptr);
    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                       sizeof(gpuCamera), &gpuCamera);
    vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffers.buffer, &offsets);
    vkCmdBindIndexBuffer(cmd, indicesBuffers.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    {
        for (const auto &draw: drawBatch) {
            VkDeviceSize indirectOffset = draw.first * sizeof(VkDrawIndexedIndirectCommand);
            uint32_t draw_stride = sizeof(VkDrawIndexedIndirectCommand);
            vkCmdDrawIndexedIndirect(cmd, frame.indirectBuffer.buffer, indirectOffset, draw.count, draw_stride);
        }
    }
    vkCmdEndRenderPass(cmd);
    VK_TRY(vkEndCommandBuffer(cmd));
    VK_TRY(vkQueueSubmit(graphicsQueue, 1, &submitInfo, frame.inFlightFences));

    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = &(swapchain.getSwapchain()),
        .pImageIndices = &imageIndex,
        .pResults = nullptr,
    };
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (vk_utils::isSwapchainInvalid(result, VK_ERROR_OUT_OF_DATE_KHR, VK_SUBOPTIMAL_KHR)) {
        return recreateSwapchain();
    }
    currentFrame = (currentFrame + 1) % MAX_FRAME_FRAME_IN_FLIGHT;
}

std::vector<VulkanApplication::DrawBatch> VulkanApplication::buildDrawBatch(std::vector<RenderObject> &object)
{
    std::sort(object.begin(), object.end(),
              [](const auto &first, const auto &second) { return first.meshID == second.meshID; });

    std::vector<DrawBatch> packedDraws;
    packedDraws.push_back({
        .meshId = object.at(0).meshID,
        .first = 0,
        .count = 1,
    });

    for (uint32_t i = 1; i < object.size(); i++) {
        if (object[i].meshID == packedDraws.back().meshId) {
            packedDraws.back().count++;
        } else {
            packedDraws.push_back({
                .meshId = object.at(i).meshID,
                .first = i,
                .count = 1,
            });
        }
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
    createInstance();
    createDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();

    swapchain.init(window->get().getSize(), physicalDevice, device, surface);

    createAllocator();
    createSyncStructure();
    createIndirectBuffer();
    createRenderPass();
    createDescriptorSetsLayout();
    createPipeline();
    createCommandPool();
    createDepthResources();
    createColorResources();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();

    this->pushModelsToGPU();
    this->pushTexturesToGPU();

    createTextureSampler();
    createTextureDescriptorSets();
    createCommandBuffers();

    materials.push_back({
        .ambientColor = {1.0f, 1.0f, 1.0f, 1.0f},
        .diffuse = {1.0f, 1.0f, 1.0f, 1.0f},
        .specular = {1.0f, 1.0f, 1.0f, 1.0f},
    });
    postInitialization();
}

void VulkanApplication::postInitialization()
{
    void *materialData = nullptr;
    for (auto &frame: frames) {
        vmaMapMemory(allocator, frame.data.materialBuffer.memory, &materialData);
        auto *objectSSBI = (gpuObject::Material *)materialData;
        for (unsigned i = 0; i < materials.size(); i++) { objectSSBI[i] = materials.at(i); }
        vmaUnmapMemory(allocator, frame.data.materialBuffer.memory);
    }
}

void VulkanApplication::recreateSwapchain()
{
    // int width = 0, height = 0;
    // glfwGetFramebufferSize(window.getWindow(), &width, &height);
    // while (width == 0 || height == 0) {
    //     glfwGetFramebufferSize(window.getWindow(), &width, &height);
    //     glfwWaitEvents();
    // }

    logger->info("Swapchain") << "Recreaing swapchain...";
    LOGGER_ENDL;

    vkDeviceWaitIdle(device);
    swapchainDeletionQueue.flush();

    swapchain.recreate(window->get().getSize(), physicalDevice, device, surface);
    createRenderPass();
    createPipeline();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createDescriptorPool();
    createDescriptorSets();
    createTextureDescriptorSets();
    createCommandBuffers();
    logger->info("Swapchain") << "Swapchain recreation complete... { height=" << swapchain.getSwapchainExtent().height
                              << ", width =" << swapchain.getSwapchainExtent().width
                              << ", numberOfImage = " << swapchain.nbOfImage() << " }";
    LOGGER_ENDL;
    postInitialization();
}
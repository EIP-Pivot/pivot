#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

namespace pivot::graphics
{

bool VulkanApplication::drawImGui(const CameraData &, const vk::CommandBufferInheritanceInfo &inheritanceInfo,
                                  vk::CommandBuffer &cmd)
{
    vk::CommandBufferBeginInfo imguiBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue,
        .pInheritanceInfo = &inheritanceInfo,
    };
    vk_utils::vk_try(cmd.begin(&imguiBeginInfo));
    vk_debug::beginRegion(cmd, "Imgui Commands", {1.f, 0.f, 0.f, 1.f});
    if (auto imguiData = ImGui::GetDrawData(); imguiData != nullptr) {
        ImGui_ImplVulkan_RenderDrawData(imguiData, cmd);
    }
    vk_debug::endRegion(cmd);
    cmd.end();
    return true;
}

bool VulkanApplication::drawScene(const CameraData &cameraData, const vk::CommandBufferInheritanceInfo &inheritanceInfo,
                                  vk::CommandBuffer &cmd)
{
    const gpu_object::VertexPushConstant vertexCamere{
        .viewProjection = cameraData.viewProjection,
    };
    const gpu_object::FragmentPushConstant fragmentCamera{
        .position = cameraData.position,
    };

    vk::DeviceSize offset = 0;
    vk::CommandBufferBeginInfo drawBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue,
        .pInheritanceInfo = &inheritanceInfo,
    };
    vk_utils::vk_try(cmd.begin(&drawBeginInfo));
    vk_debug::beginRegion(cmd, "Draw Commands", {0.f, 1.f, 0.f, 1.f});
    if (drawResolver.getFrameData(currentFrame).packedDraws.size() > 0) {
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0,
                               drawResolver.getFrameData(currentFrame).objectDescriptor, nullptr);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, ressourceDescriptorSet, nullptr);
        cmd.pushConstants<gpu_object::VertexPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0,
                                                          vertexCamere);
        cmd.pushConstants<gpu_object::FragmentPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eFragment,
                                                            sizeof(gpu_object::VertexPushConstant), fragmentCamera);
        cmd.bindVertexBuffers(0, assetStorage.getVertexBuffer().buffer, offset);
        cmd.bindIndexBuffer(assetStorage.getIndexBuffer().buffer, 0, vk::IndexType::eUint32);

        for (const auto &packedPipeline: drawResolver.getFrameData(currentFrame).pipelineBatch) {
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineStorage.getGraphics(packedPipeline.pipelineID));

            if (deviceFeature.multiDrawIndirect == VK_TRUE) {
                cmd.drawIndexedIndirect(drawResolver.getFrameData(currentFrame).indirectBuffer.buffer,
                                        packedPipeline.first * sizeof(vk::DrawIndexedIndirectCommand),
                                        packedPipeline.size, sizeof(vk::DrawIndexedIndirectCommand));
            } else {
                for (auto i = packedPipeline.first; i < packedPipeline.size; i++) {
                    const auto &draw = drawResolver.getFrameData(currentFrame).packedDraws.at(i);
                    cmd.drawIndexedIndirect(drawResolver.getFrameData(currentFrame).indirectBuffer.buffer,
                                            draw.first * sizeof(vk::DrawIndexedIndirectCommand), draw.count,
                                            sizeof(vk::DrawIndexedIndirectCommand));
                }
            }
        }
    }
    vk_debug::endRegion(cmd);
    cmd.end();
    return true;
}

bool VulkanApplication::dispatchCulling(const CameraData &cameraData, const vk::CommandBufferInheritanceInfo &,
                                        vk::CommandBuffer &cmd)
{
    const gpu_object::CullingPushConstant cullingCamera{
        .viewProjection = cameraData.viewProjection,
        .drawCount = static_cast<uint32_t>(drawResolver.getFrameData(currentFrame).packedDraws.size()),
    };

    vk_debug::beginRegion(cmd, "culling pass", {1.f, 0.f, 1.f, 1.f});
    vk::BufferMemoryBarrier barrier{
        .srcAccessMask = vk::AccessFlagBits::eIndirectCommandRead,
        .dstAccessMask = vk::AccessFlagBits::eShaderWrite,
        .srcQueueFamilyIndex = queueIndices.graphicsFamily.value(),
        .dstQueueFamilyIndex = queueIndices.graphicsFamily.value(),
        .buffer = drawResolver.getFrameData(currentFrame).indirectBuffer.buffer,
        .size = drawResolver.getFrameData(currentFrame).indirectBuffer.getSize(),
    };
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, cullingLayout, 0,
                           drawResolver.getFrameData(currentFrame).objectDescriptor, nullptr);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, cullingLayout, 1, ressourceDescriptorSet, nullptr);
    cmd.pushConstants<gpu_object::CullingPushConstant>(cullingLayout, vk::ShaderStageFlagBits::eCompute, 0,
                                                       cullingCamera);
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipelineStorage.getCompute("culling"));
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eDrawIndirect, vk::PipelineStageFlagBits::eComputeShader, {}, {},
                        barrier, {});
    cmd.dispatch((drawResolver.getFrameData(currentFrame).packedDraws.size() / 256) + 1, 1, 1);

    barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eIndirectCommandRead;
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eDrawIndirect, {}, {},
                        barrier, {});
    vk_debug::endRegion(cmd);
    return true;
}

}    // namespace pivot::graphics

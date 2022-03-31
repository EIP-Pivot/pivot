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

    vk::CommandBufferBeginInfo drawBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue,
        .pInheritanceInfo = &inheritanceInfo,
    };
    vk_utils::vk_try(cmd.begin(&drawBeginInfo));
    vk_debug::beginRegion(cmd, "Draw Commands", {0.f, 1.f, 0.f, 1.f});
    assetStorage.bindForGraphics(cmd, pipelineLayout);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1,
                           drawResolver.getFrameData(currentFrame).objectDescriptor, nullptr);
    cmd.pushConstants<gpu_object::VertexPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0,
                                                      vertexCamere);
    cmd.pushConstants<gpu_object::FragmentPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eFragment,
                                                        sizeof(gpu_object::VertexPushConstant), fragmentCamera);
    for (const auto &packedPipeline: drawResolver.getFrameData(currentFrame).pipelineBatch) {
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineStorage.get(packedPipeline.pipelineID));

        if (deviceFeature.multiDrawIndirect == VK_TRUE) {
            cmd.drawIndexedIndirect(drawResolver.getFrameData(currentFrame).indirectBuffer.buffer,
                                    packedPipeline.first * sizeof(vk::DrawIndexedIndirectCommand), packedPipeline.size,
                                    sizeof(vk::DrawIndexedIndirectCommand));
        } else {
            for (auto i = packedPipeline.first; i < packedPipeline.size; i++) {
                const auto &draw = drawResolver.getFrameData(currentFrame).packedDraws.at(i);
                cmd.drawIndexedIndirect(drawResolver.getFrameData(currentFrame).indirectBuffer.buffer,
                                        draw.first * sizeof(vk::DrawIndexedIndirectCommand), draw.count,
                                        sizeof(vk::DrawIndexedIndirectCommand));
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
    assetStorage.bindForCompute(cmd, cullingLayout);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, cullingLayout, 1,
                           drawResolver.getFrameData(currentFrame).objectDescriptor, nullptr);
    cmd.pushConstants<gpu_object::CullingPushConstant>(cullingLayout, vk::ShaderStageFlagBits::eCompute, 0,
                                                       cullingCamera);
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipelineStorage.get("culling"));
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

bool VulkanApplication::drawText(const CameraData &cameraData, const vk::CommandBufferInheritanceInfo &info,
                                 const std::vector<std::pair<glm::ivec2, std::string>> &text, vk::CommandBuffer &cmd)
{
    vk::DeviceSize offset = 0;
    const gpu_object::VertexPushConstant vertexCamere{
        .viewProjection = cameraData.projection,
    };
    vk::CommandBufferBeginInfo textBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue,
        .pInheritanceInfo = &info,
    };
    vk_utils::vk_try(cmd.begin(&textBeginInfo));
    vk_debug::beginRegion(cmd, "Text Commands", {0.2f, 0.2f, 0.2f, 1.f});
    if (!text.empty()) {
        cmd.pushConstants<gpu_object::VertexPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0,
                                                          vertexCamere);
        assetStorage.bindForGraphics(cmd, pipelineLayout, 0, false, false);
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineStorage.get("font"));
        auto vert = textBuffer.getMappedPointer<float>();

        glm::vec2 cursor = text.at(0).first * 5;
        for (const auto &[position, line]: text) {
            logger.debug("VulkanApplication") << line;
            for (const auto &c: line) {
                auto character = assetStorage.getChar(c);
                const auto &texture = assetStorage.getTextures().get(character.textureId);
                glm::vec2 pos = {
                    cursor.x + character.bearing.x,
                    cursor.y - (texture.size.height - character.bearing.y),
                };
                glm::ivec2 size = {texture.size.width, texture.size.height};
                size *= 20;
                pos *= 20;
                float verticies[6][4] = {
                    {pos.x, pos.y + size.y, 0.0f, 0.0f},          {pos.x, pos.y, 0.0f, 1.0f},
                    {pos.x + size.x, pos.y, 1.0f, 1.0f},

                    {pos.x, pos.y + size.y, 0.0f, 0.0f},          {pos.x + size.x, pos.y, 1.0f, 1.0f},
                    {pos.x + size.x, pos.y + size.y, 1.0f, 0.0f},
                };
                std::memcpy(vert, verticies, sizeof(float) * 6 * 4);
                cmd.bindVertexBuffers(0, textBuffer.buffer, offset);
                cmd.draw(6, 1, 0, 0);
                cursor += (character.advance >> 6);
            }
        }
    }
    cmd.end();
    vk_debug::endRegion(cmd);
    return true;
}

}    // namespace pivot::graphics

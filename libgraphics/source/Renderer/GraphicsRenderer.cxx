#include "pivot/graphics/Renderer/GraphicsRenderer.hxx"

#include "pivot/graphics/PipelineBuilders/GraphicsPipelineBuilder.hxx"
#include "pivot/graphics/vk_debug.hxx"

namespace pivot::graphics
{

GraphicsRenderer::GraphicsRenderer(PipelineStorage &storage, AssetStorage &assets): IGraphicsRenderer(storage, assets)
{
}
GraphicsRenderer::~GraphicsRenderer() {}

bool GraphicsRenderer::onInit(const vk::Extent2D &size, VulkanBase &base_ref,
                              const vk::DescriptorSetLayout &resolverLayout, vk::RenderPass &pass)
{
    bIsMultiDraw = base_ref.deviceFeature.multiDrawIndirect;
    createPipelineLayout(base_ref.device, resolverLayout);
    createPipeline(base_ref, pass, size);
    return true;
}

void GraphicsRenderer::onStop(VulkanBase &base_ref)
{
    if (pipelineLayout) base_ref.device.destroyPipelineLayout(pipelineLayout);
}

bool GraphicsRenderer::onRecreate(const vk::Extent2D &size, VulkanBase &base_ref,
                                  const vk::DescriptorSetLayout &resolverLayout, vk::RenderPass &pass)
{
    onStop(base_ref);
    stor.removePipeline("pbr");
    stor.removePipeline("lit");
    stor.removePipeline("unlit");
    stor.removePipeline("wireframe");
    stor.removePipeline("skybox");
    return onInit(size, base_ref, resolverLayout, pass);
}

bool GraphicsRenderer::onDraw(const CameraData &cameraData, DrawCallResolver &resolver, vk::CommandBuffer &cmd)
{
    const gpu_object::VertexPushConstant vertexCamere{
        .viewProjection = cameraData.viewProjection,
    };
    const gpu_object::FragmentPushConstant fragmentCamera{
        .omniLightCount = static_cast<uint32_t>(resolver.getFrameData().pointLightCount),
        .directLightCount = static_cast<uint32_t>(resolver.getFrameData().directionalLightCount),
        .spotLightCount = static_cast<uint32_t>(resolver.getFrameData().spotLightCount),
        .position = cameraData.position,
    };

    vk_debug::beginRegion(cmd, "Draw Commands", {0.f, 1.f, 0.f, 1.f});
    assets.bindForGraphics(cmd, pipelineLayout);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1,
                           resolver.getFrameData().objectDescriptor, nullptr);
    cmd.pushConstants<gpu_object::VertexPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0,
                                                      vertexCamere);
    cmd.pushConstants<gpu_object::FragmentPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eFragment,
                                                        sizeof(gpu_object::VertexPushConstant), fragmentCamera);
    for (const auto &packedPipeline: resolver.getFrameData().pipelineBatch) {
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, stor.getGraphics(packedPipeline.pipelineID));

        if (bIsMultiDraw) {
            cmd.drawIndexedIndirect(resolver.getFrameData().indirectBuffer.buffer,
                                    packedPipeline.first * sizeof(vk::DrawIndexedIndirectCommand), packedPipeline.size,
                                    sizeof(vk::DrawIndexedIndirectCommand));
        } else {
            for (auto i = packedPipeline.first; i < packedPipeline.size; i++) {
                const auto &draw = resolver.getFrameData().packedDraws.at(i);
                cmd.drawIndexedIndirect(resolver.getFrameData().indirectBuffer.buffer,
                                        draw.first * sizeof(vk::DrawIndexedIndirectCommand), draw.count,
                                        sizeof(vk::DrawIndexedIndirectCommand));
            }
        }
    }
    vk_debug::endRegion(cmd);
    return true;
}

void GraphicsRenderer::createPipelineLayout(vk::Device &device, const vk::DescriptorSetLayout &resolverLayout)
{
    DEBUG_FUNCTION
    std::vector<vk::PushConstantRange> pipelinePushConstant = {
        vk_init::populateVkPushConstantRange(vk::ShaderStageFlagBits::eVertex, sizeof(gpu_object::VertexPushConstant)),
        vk_init::populateVkPushConstantRange(vk::ShaderStageFlagBits::eFragment,
                                             sizeof(gpu_object::FragmentPushConstant),
                                             sizeof(gpu_object::VertexPushConstant)),
    };

    std::vector<vk::DescriptorSetLayout> setLayout = {assets.getDescriptorSetLayout(), resolverLayout};
    auto pipelineLayoutCreateInfo = vk_init::populateVkPipelineLayoutCreateInfo(setLayout, pipelinePushConstant);
    pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
    vk_debug::setObjectName(device, pipelineLayout, "Graphics pipeline Layout");
}

void GraphicsRenderer::createPipeline(VulkanBase &base_ref, vk::RenderPass &pass, const vk::Extent2D &size)
{
    DEBUG_FUNCTION

    GraphicsPipelineBuilder builder(size);
    builder.setPipelineLayout(pipelineLayout)
        .setRenderPass(pass)
        .setMsaaSample(base_ref.maxMsaaSample)
        .setFaceCulling(vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise)
        .setVertexShaderPath("shaders/default_pbr.vert.spv")
        .setFragmentShaderPath("shaders/default_pbr.frag.spv");
    stor.newGraphicsPipeline("pbr", builder);
    stor.setDefault("pbr");

    builder.setVertexShaderPath("shaders/default.vert.spv").setFragmentShaderPath("shaders/default_lit.frag.spv");
    stor.newGraphicsPipeline("lit", builder);

    builder.setFragmentShaderPath("shaders/default_unlit.frag.spv");
    stor.newGraphicsPipeline("unlit", builder);

    builder.setPolygonMode(vk::PolygonMode::eLine);
    stor.newGraphicsPipeline("wireframe", builder);

    builder.setPolygonMode(vk::PolygonMode::eFill);
    builder.setFaceCulling(vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise);
    stor.newGraphicsPipeline("skybox", builder);
}

}    // namespace pivot::graphics

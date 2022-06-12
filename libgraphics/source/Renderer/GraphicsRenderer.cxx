#include "pivot/graphics/Renderer/GraphicsRenderer.hxx"

#include "pivot/graphics/PipelineBuilders/GraphicsPipelineBuilder.hxx"
#include "pivot/graphics/vk_debug.hxx"

namespace pivot::graphics
{

GraphicsRenderer::GraphicsRenderer(StorageUtils &utils): IGraphicsRenderer(utils) {}
GraphicsRenderer::~GraphicsRenderer() {}

bool GraphicsRenderer::onInit(const vk::Extent2D &size, VulkanBase &base_ref,
                              const vk::DescriptorSetLayout &resolverLayout, vk::RenderPass &pass)
{
    bIsMultiDraw = base_ref.deviceFeature.multiDrawIndirect;
    viewport = vk::Viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(size.width),
        .height = static_cast<float>(size.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    scissor = vk::Rect2D{
        .extent = size,
    };
    createPipelineLayout(base_ref.device, resolverLayout);
    createPipeline(base_ref, pass);
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
    storage.pipeline.get().removePipeline("pbr");
    storage.pipeline.get().removePipeline("lit");
    storage.pipeline.get().removePipeline("unlit");
    storage.pipeline.get().removePipeline("wireframe");
    storage.pipeline.get().removePipeline("skybox");
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
    storage.assets.get().bindForGraphics(cmd, pipelineLayout);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1,
                           resolver.getFrameData().objectDescriptor, nullptr);
    cmd.pushConstants<gpu_object::VertexPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0,
                                                      vertexCamere);
    cmd.pushConstants<gpu_object::FragmentPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eFragment,
                                                        sizeof(gpu_object::VertexPushConstant), fragmentCamera);
    for (const auto &packedPipeline: resolver.getFrameData().pipelineBatch) {
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                         storage.pipeline.get().getGraphics(packedPipeline.pipelineID));
        cmd.setViewport(0, viewport);
        cmd.setScissor(0, scissor);

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

    std::vector<vk::DescriptorSetLayout> setLayout = {storage.assets.get().getDescriptorSetLayout(), resolverLayout};
    auto pipelineLayoutCreateInfo = vk_init::populateVkPipelineLayoutCreateInfo(setLayout, pipelinePushConstant);
    pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
    vk_debug::setObjectName(device, pipelineLayout, "Graphics pipeline Layout");
}

void GraphicsRenderer::createPipeline(VulkanBase &base_ref, vk::RenderPass &pass)
{
    DEBUG_FUNCTION

    GraphicsPipelineBuilder builder;
    const std::uint32_t count = storage.assets.get().getSize<AssetStorage::Texture>();
    std::vector<vk::SpecializationMapEntry> specializationData = {
        {
            .constantID = 0,
            .offset = 0,
            .size = sizeof(std::uint32_t),
        },
    };

    builder.setPipelineLayout(pipelineLayout)
        .setRenderPass(pass)
        .setMsaaSample(base_ref.maxMsaaSample)
        .setFaceCulling(vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise);

    builder.setVertexShaderPath("shaders/default.vert.spv").setFragmentShaderPath("shaders/default_lit.frag.spv");
    storage.pipeline.get().newGraphicsPipeline(
        "lit", builder.buildWithSpecialisation(base_ref.device, specializationData, count,
                                               vk::ShaderStageFlagBits::eFragment, storage.pipeline.get().getCache()));

    builder.setFragmentShaderPath("shaders/default_unlit.frag.spv");
    storage.pipeline.get().newGraphicsPipeline(
        "unlit",
        builder.buildWithSpecialisation(base_ref.device, specializationData, count, vk::ShaderStageFlagBits::eFragment,
                                        storage.pipeline.get().getCache()));

    builder.setPolygonMode(vk::PolygonMode::eLine);
    storage.pipeline.get().newGraphicsPipeline(
        "wireframe",
        builder.buildWithSpecialisation(base_ref.device, specializationData, count, vk::ShaderStageFlagBits::eFragment,
                                        storage.pipeline.get().getCache()));

    builder.setPolygonMode(vk::PolygonMode::eFill);
    builder.setFaceCulling(vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise);
    storage.pipeline.get().newGraphicsPipeline(
        "skybox",
        builder.buildWithSpecialisation(base_ref.device, specializationData, count, vk::ShaderStageFlagBits::eFragment,
                                        storage.pipeline.get().getCache()));

    builder.setFaceCulling(vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise)
        .setVertexAttributes(Vertex::getInputAttributeDescriptions(
            0, VertexComponentFlagBits::Position | VertexComponentFlagBits::Normal | VertexComponentFlagBits::UV |
                   VertexComponentFlagBits::Tangent))
        .setVertexShaderPath("shaders/default_pbr.vert.spv")
        .setFragmentShaderPath("shaders/default_pbr.frag.spv");
    storage.pipeline.get().newGraphicsPipeline(
        "pbr", builder.buildWithSpecialisation(base_ref.device, specializationData, count,
                                               vk::ShaderStageFlagBits::eFragment, storage.pipeline.get().getCache()));
    storage.pipeline.get().setDefault("pbr");
}

}    // namespace pivot::graphics

#include <cpplogger/Logger.hpp>
#include <pivot/config.hxx>

#include "pivot/graphics/Renderer/GraphicsRenderer.hxx"

#include "pivot/graphics/PipelineBuilders/GraphicsPipelineBuilder.hxx"
#include "pivot/graphics/vk_debug.hxx"

#include "pivot/graphics/Resolver/DrawCallResolver.hxx"
#include "pivot/graphics/Resolver/LightDataResolver.hxx"

namespace pivot::graphics
{

GraphicsRenderer::GraphicsRenderer(StorageUtils &utils): IGraphicsRenderer(utils) {}
GraphicsRenderer::~GraphicsRenderer() {}

bool GraphicsRenderer::onInit(const vk::Extent2D &, VulkanBase &base_ref, const ResolverDispatcher &dispatcher,
                              vk::RenderPass &pass)
{
    DEBUG_FUNCTION();

    bIsMultiDraw = base_ref.deviceFeature.multiDrawIndirect;

    createPipelineLayout(base_ref.device, dispatcher);
    createPipeline(base_ref, pass);
    return true;
}

void GraphicsRenderer::onStop(VulkanBase &base_ref)
{
    DEBUG_FUNCTION();

    storage.pipeline.get().removePipeline("pbr");
    storage.pipeline.get().removePipeline("lit");
    storage.pipeline.get().removePipeline("unlit");
    storage.pipeline.get().removePipeline("wireframe");
    storage.pipeline.get().removePipeline("skybox");
    if (pipelineLayout) base_ref.device.destroyPipelineLayout(pipelineLayout);
}

bool GraphicsRenderer::onDraw(const RenderingContext &context, const CameraData &cameraData,
                              ResolverDispatcher &dispatcher, vk::CommandBuffer &cmd)
{
    PROFILE_FUNCTION();

    const LightDataResolver &light = dispatcher.get<LightDataResolver>();
    const DrawCallResolver &resolver = dispatcher.get<DrawCallResolver>();

    const gpu_object::VertexPushConstant vertexCamere{
        .viewProjection = cameraData.viewProjection,
    };
    const gpu_object::FragmentPushConstant fragmentCamera{
        .omniLightCount = static_cast<uint32_t>(light.getFrameData().pointLightCount),
        .directLightCount = static_cast<uint32_t>(light.getFrameData().directionalLightCount),
        .spotLightCount = static_cast<uint32_t>(light.getFrameData().spotLightCount),
        .position = cameraData.position,
    };
    const vk::Viewport viewport{
        .x = static_cast<float>(context.renderArea.offset.x),
        .y = static_cast<float>(context.renderArea.offset.y),
        .width = static_cast<float>(context.renderArea.extent.width),
        .height = static_cast<float>(context.renderArea.extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    vk_debug::beginRegion(cmd, "Draw Commands", {0.f, 1.f, 0.f, 1.f});
    dispatcher.bind(cmd, pipelineLayout, vk::PipelineBindPoint::eGraphics);
    cmd.pushConstants<gpu_object::VertexPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0,
                                                      vertexCamere);
    cmd.pushConstants<gpu_object::FragmentPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eFragment,
                                                        sizeof(gpu_object::VertexPushConstant), fragmentCamera);
    for (const auto &packedPipeline: resolver.getFrameData().pipelineBatch) {
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                         storage.pipeline.get().getGraphics(packedPipeline.pipelineID));
        cmd.setViewport(0, viewport);
        cmd.setScissor(0, context.renderArea);

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

void GraphicsRenderer::createPipelineLayout(vk::Device &device, const ResolverDispatcher &dispatcher)
{
    DEBUG_FUNCTION();
    std::vector<vk::PushConstantRange> pipelinePushConstant = {
        vk_init::populateVkPushConstantRange(vk::ShaderStageFlagBits::eVertex, sizeof(gpu_object::VertexPushConstant)),
        vk_init::populateVkPushConstantRange(vk::ShaderStageFlagBits::eFragment,
                                             sizeof(gpu_object::FragmentPushConstant),
                                             sizeof(gpu_object::VertexPushConstant)),
    };

    std::vector<vk::DescriptorSetLayout> setLayout = dispatcher.getDescriptorPair();

    auto pipelineLayoutCreateInfo = vk_init::populateVkPipelineLayoutCreateInfo(setLayout, pipelinePushConstant);
    pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
    vk_debug::setObjectName(device, pipelineLayout, "Graphics pipeline Layout");
}

void GraphicsRenderer::createPipeline(VulkanBase &base_ref, vk::RenderPass &pass)
{
    DEBUG_FUNCTION();

    GraphicsPipelineBuilder builder;
    const std::uint32_t count = storage.assets.get().getSize<AssetStorage::Texture>();
    std::vector<vk::SpecializationMapEntry> specializationData = {
        {
            .constantID = 0,
            .offset = 0,
            .size = sizeof(std::uint32_t),
        },
    };

    std::filesystem::path shader_directory = pivot::Config::find_shaders_folder();

    builder.setPipelineLayout(pipelineLayout)
        .setRenderPass(pass)
        .setMsaaSample(base_ref.maxMsaaSample)
        .setFaceCulling(vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise);

    builder.setVertexShaderPath(shader_directory / "default.vert.spv")
        .setFragmentShaderPath(shader_directory / "default_lit.frag.spv");
    storage.pipeline.get().newGraphicsPipeline(
        "lit", builder.buildWithSpecialisation(base_ref.device, specializationData, count,
                                               vk::ShaderStageFlagBits::eFragment, storage.pipeline.get().getCache()));

    builder.setFragmentShaderPath(shader_directory / "default_unlit.frag.spv");
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
                   VertexComponentFlagBits::Color | VertexComponentFlagBits::Tangent))
        .setVertexShaderPath(shader_directory / "default_pbr.vert.spv")
        .setFragmentShaderPath(shader_directory / "default_pbr.frag.spv");
    storage.pipeline.get().newGraphicsPipeline(
        "pbr", builder.buildWithSpecialisation(base_ref.device, specializationData, count,
                                               vk::ShaderStageFlagBits::eFragment, storage.pipeline.get().getCache()));
    storage.pipeline.get().setDefault("pbr");
}

}    // namespace pivot::graphics

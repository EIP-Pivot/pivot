#include "pivot/graphics/Renderer/CullingRenderer.hxx"

#include "pivot/graphics/PipelineBuilders/ComputePipelineBuilder.hxx"
#include "pivot/graphics/vk_debug.hxx"

namespace pivot::graphics
{

CullingRenderer::CullingRenderer(StorageUtils &utils): IComputeRenderer(utils) {}
CullingRenderer::~CullingRenderer() {}

bool CullingRenderer::onInit(VulkanBase &base_ref, const ResolverDispatcher &dispatcher)
{
    DEBUG_FUNCTION;
    indices = base_ref.queueIndices;
    createPipelineLayout(base_ref.device, dispatcher);
    createPipeline();
    return true;
}

void CullingRenderer::onStop(VulkanBase &base_ref)
{
    DEBUG_FUNCTION;
    storage.pipeline.get().removePipeline("culling");
    if (cullingLayout) base_ref.device.destroyPipelineLayout(cullingLayout);
}

bool CullingRenderer::onDraw(const RenderingContext &, const CameraData &cameraData, ResolverDispatcher &dispatcher,
                             vk::CommandBuffer &cmd)
{
    const DrawCallResolver &resolver = dispatcher.get<DrawCallResolver>();
    const gpu_object::CullingPushConstant cullingCamera{
        .viewProjection = cameraData.viewProjection,
        .drawCount = static_cast<uint32_t>(resolver.getFrameData().packedDraws.size()),
    };

    vk::BufferMemoryBarrier barrier{
        .srcAccessMask = vk::AccessFlagBits::eIndirectCommandRead,
        .dstAccessMask = vk::AccessFlagBits::eShaderWrite,
        .srcQueueFamilyIndex = indices.graphicsFamily.value(),
        .dstQueueFamilyIndex = indices.graphicsFamily.value(),
        .buffer = resolver.getFrameData().indirectBuffer.buffer,
        .size = resolver.getFrameData().indirectBuffer.getSize(),
    };

    vk_debug::beginRegion(cmd, "Culling pass", {1.f, 0.f, 1.f, 1.f});
    storage.assets.get().bindForCompute(cmd, cullingLayout, 2);
    dispatcher.bind(cmd, cullingLayout, vk::PipelineBindPoint::eCompute);
    cmd.pushConstants<gpu_object::CullingPushConstant>(cullingLayout, vk::ShaderStageFlagBits::eCompute, 0,
                                                       cullingCamera);
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, storage.pipeline.get().getCompute("culling"));
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eDrawIndirect, vk::PipelineStageFlagBits::eComputeShader, {}, {},
                        barrier, {});
    cmd.dispatch((resolver.getFrameData().packedDraws.size() / 256) + 1, 1, 1);

    barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eIndirectCommandRead;
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eDrawIndirect, {}, {},
                        barrier, {});
    vk_debug::endRegion(cmd);
    return true;
}

void CullingRenderer::createPipelineLayout(vk::Device &device, const ResolverDispatcher &dispatcher)
{
    DEBUG_FUNCTION
    std::vector<vk::PushConstantRange> pipelinePushConstant = {vk_init::populateVkPushConstantRange(
        vk::ShaderStageFlagBits::eCompute, sizeof(gpu_object::CullingPushConstant))};

    std::vector<vk::DescriptorSetLayout> setLayout(dispatcher.getDescriptorPair());
    setLayout.push_back(storage.assets.get().getDescriptorSetLayout());

    auto pipelineLayoutCreateInfo = vk_init::populateVkPipelineLayoutCreateInfo(setLayout, pipelinePushConstant);
    cullingLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
    vk_debug::setObjectName(device, cullingLayout, "Culling pipeline Layout");
}

void CullingRenderer::createPipeline()
{
    DEBUG_FUNCTION
    ComputePipelineBuilder builder;
    builder.setPipelineLayout(cullingLayout).setComputeShaderPath("shaders/culling.comp.spv");
    storage.pipeline.get().newComputePipeline("culling", builder);
}

}    // namespace pivot::graphics

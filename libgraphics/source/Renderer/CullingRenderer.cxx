#include "pivot/graphics/Renderer/CullingRenderer.hxx"

#include "pivot/graphics/PipelineBuilders/ComputePipelineBuilder.hxx"
#include "pivot/graphics/vk_debug.hxx"

namespace pivot::graphics
{

CullingRenderer::CullingRenderer(PipelineStorage &storage, AssetStorage &assets): IComputeRenderer(storage, assets) {}
CullingRenderer::~CullingRenderer() {}

bool CullingRenderer::onInit(VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout)
{
    indices = base_ref.queueIndices;
    createPipelineLayout(base_ref.device, resolverLayout);
    createPipeline(base_ref.device);
    return true;
}

void CullingRenderer::onStop(VulkanBase &base_ref)
{
    if (cullingLayout) base_ref.device.destroyPipelineLayout(cullingLayout);
}

bool CullingRenderer::onDraw(const CameraData &cameraData, DrawCallResolver &resolver, vk::CommandBuffer &cmd)
{
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

    vk_debug::beginRegion(cmd, "culling pass", {1.f, 0.f, 1.f, 1.f});
    assets.bindForCompute(cmd, cullingLayout);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, cullingLayout, 1, resolver.getFrameData().objectDescriptor,
                           nullptr);
    cmd.pushConstants<gpu_object::CullingPushConstant>(cullingLayout, vk::ShaderStageFlagBits::eCompute, 0,
                                                       cullingCamera);
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, stor.getCompute("culling"));
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

void CullingRenderer::createPipelineLayout(vk::Device &device, vk::DescriptorSetLayout &resolverLayout)
{
    DEBUG_FUNCTION
    std::vector<vk::PushConstantRange> pipelinePushConstant = {vk_init::populateVkPushConstantRange(
        vk::ShaderStageFlagBits::eCompute, sizeof(gpu_object::CullingPushConstant))};
    std::vector<vk::DescriptorSetLayout> setLayout = {assets.getDescriptorSetLayout(), resolverLayout};
    auto pipelineLayoutCreateInfo = vk_init::populateVkPipelineLayoutCreateInfo(setLayout, pipelinePushConstant);
    cullingLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
    vk_debug::setObjectName(device, cullingLayout, "Culling pipeline Layout");
}

void CullingRenderer::createPipeline(vk::Device &device)
{
    DEBUG_FUNCTION
    ComputePipelineBuilder builder;
    builder.setPipelineLayout(cullingLayout).setComputeShaderPath("shaders/culling.comp.spv");
    stor.newComputePipeline("culling", builder);
}

}    // namespace pivot::graphics

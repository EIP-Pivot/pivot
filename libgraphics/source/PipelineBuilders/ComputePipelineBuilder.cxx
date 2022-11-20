#include "pivot/graphics/PipelineBuilders/ComputePipelineBuilder.hxx"

#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics
{

ComputePipelineBuilder &ComputePipelineBuilder::setPipelineLayout(vk::PipelineLayout &layout)
{
    pipelineLayout = layout;
    return *this;
}

ComputePipelineBuilder &ComputePipelineBuilder::setComputeShaderPath(const std::filesystem::path &path)
{
    shaderPath = path;
    return *this;
}

vk::Pipeline ComputePipelineBuilder::build(vk::Device &device, vk::PipelineCache pipelineCache) const
{
    auto computeShaderCode = vk_utils::readBinaryFile(shaderPath);
    auto computeShaderModule = vk_utils::createShaderModule(device, computeShaderCode);
    auto computeShaderStage =
        vk_init::populateVkPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits::eCompute, computeShaderModule);
    vk_debug::setObjectName(device, computeShaderModule, shaderPath);

    vk::ComputePipelineCreateInfo pipelineInfo{
        .stage = computeShaderStage,
        .layout = pipelineLayout,
    };

    vk::Pipeline newPipeline{};
    vk::Result result;
    std::tie(result, newPipeline) = device.createComputePipeline(pipelineCache, pipelineInfo);
    device.destroy(computeShaderModule);
    vk_utils::vk_try(result);
    return newPipeline;
}

}    // namespace pivot::graphics

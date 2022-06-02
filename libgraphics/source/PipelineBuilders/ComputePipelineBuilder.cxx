#include "pivot/graphics/PipelineBuilders/ComputePipelineBuilder.hxx"

#include "pivot/graphics/VulkanShader.hxx"
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

ComputePipelineBuilder &ComputePipelineBuilder::setComputeShaderPath(const std::string &path)
{
    shaderPath = path;
    return *this;
}

vk::Pipeline ComputePipelineBuilder::build(vk::Device &device, vk::PipelineCache pipelineCache) const
{
    auto shaderId = shader.load(shaderPath);
    auto shaderModule = vk_utils::createShaderModule(device, shader.get(shaderId).getByteCode());
    vk_debug::setObjectName(device, shaderModule, shaderPath);
    auto computeShaderStage =
        vk_init::populateVkPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits::eCompute, shaderModule);
    vk::ComputePipelineCreateInfo pipelineInfo{
        .stage = computeShaderStage,
        .layout = pipelineLayout,
    };

    vk::Pipeline newPipeline{};
    vk::Result result;
    std::tie(result, newPipeline) = device.createComputePipeline(pipelineCache, pipelineInfo);
    device.destroy(shaderModule);
    vk_utils::vk_try(result);
    return newPipeline;
}

}    // namespace pivot::graphics

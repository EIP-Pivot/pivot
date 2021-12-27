#pragma once

#include "pivot/graphics/interface/IPipelineBuilder.hxx"

namespace pivot::graphics
{

class ComputePipelineBuilder final : public IPipelineBuilder
{
public:
    ComputePipelineBuilder() = default;
    ~ComputePipelineBuilder() = default;

    ComputePipelineBuilder &setPipelineLayout(vk::PipelineLayout &layout);
    ComputePipelineBuilder &setComputeShaderPath(const std::string &);

    vk::Pipeline build(vk::Device &, vk::PipelineCache) const;

private:
    std::string shaderPath;
    vk::PipelineLayout pipelineLayout;
};

}    // namespace pivot::graphics
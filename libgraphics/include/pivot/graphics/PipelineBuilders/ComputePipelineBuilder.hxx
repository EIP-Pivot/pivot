#pragma once

#include "pivot/graphics/interface/IPipelineBuilder.hxx"
#include <string>

namespace pivot::graphics
{

///@class ComputePipelineBuilder
///@brief Utility class to ease the creation of Vulkan compute pipelines
class ComputePipelineBuilder final : public interface::IPipelineBuilder
{
public:
    ComputePipelineBuilder() = default;
    ~ComputePipelineBuilder() = default;

    const char *const getDebugPipelineName() const noexcept { return "Compute Pipeline"; }

    ComputePipelineBuilder &setPipelineLayout(vk::PipelineLayout &layout);

    /// Set the path of the shader to use
    ComputePipelineBuilder &setComputeShaderPath(const std::string &);

    vk::Pipeline build(vk::Device &, vk::PipelineCache) const;

private:
    std::string shaderPath;
    vk::PipelineLayout pipelineLayout;
};

}    // namespace pivot::graphics
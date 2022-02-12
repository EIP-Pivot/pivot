#pragma once

#include "pivot/graphics/interface/IPipelineBuilder.hxx"
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// @class GraphicsPipelineBuilder
///
/// @brief Utility class to ease the creation of Vulkan pipelines
class GraphicsPipelineBuilder final : public interface::IPipelineBuilder
{
public:
    GraphicsPipelineBuilder() = delete;
    /// Construct a pipeline with a predefined extent
    GraphicsPipelineBuilder(const vk::Extent2D &);
    ~GraphicsPipelineBuilder();

    constexpr const char *getDebugPipelineName() const noexcept { return "Graphics Pipeline"; }

    GraphicsPipelineBuilder &setPipelineLayout(vk::PipelineLayout &);

    /// Set the renderPass use by the pipeline, can be chained
    GraphicsPipelineBuilder &setRenderPass(vk::RenderPass &);

    /// Set the path of the vertex shader to use, can be chained
    GraphicsPipelineBuilder &setVertexShaderPath(const std::string &);
    /// Set the path of the fragment shader to use, can be chained
    GraphicsPipelineBuilder &setFragmentShaderPath(const std::string &);
    /// Set the amount of super sampling, can be chained
    GraphicsPipelineBuilder &setMsaaSample(const vk::SampleCountFlagBits &);
    /// Set the rasterizer' polygon mode, can be chained
    GraphicsPipelineBuilder &setPolygonMode(const vk::PolygonMode &);
    /// Set the rasterizer' face culling config, can be chained
    GraphicsPipelineBuilder &setFaceCulling(const vk::CullModeFlags &, const vk::FrontFace &);

    /// Build the Vulkan pipeline
    ///
    /// @return A valid Vulkan pipeline, or VK_NULL_REFERENCE if an error has occurred
    vk::Pipeline build(vk::Device &device, vk::PipelineCache pipelineCache = VK_NULL_HANDLE) const;

private:
    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    vk::PipelineMultisampleStateCreateInfo multisampling;
    vk::PipelineDepthStencilStateCreateInfo depthStencil;
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    vk::Viewport viewport;
    vk::Rect2D scissor;
    vk::PipelineLayout pipelineLayout;
    vk::RenderPass renderPass;
};

}    // namespace pivot::graphics

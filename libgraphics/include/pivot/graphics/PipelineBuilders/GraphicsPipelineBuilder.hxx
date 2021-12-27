#pragma once

#include "pivot/graphics/interface/IPipelineBuilder.hxx"
#include "pivot/graphics/types/Vertex.hxx"
#include "pivot/graphics/vk_init.hxx"

#include <vector>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// @class GraphicsPipelineBuilder
///
/// @brief Utility class to ease the creation of Vulkan pipelines
/// You must provide and fill all the member variable before calling the build() method
class GraphicsPipelineBuilder final : public IPipelineBuilder
{
public:
    GraphicsPipelineBuilder();
    GraphicsPipelineBuilder(const vk::Extent2D &);
    ~GraphicsPipelineBuilder();

    /// Set the layout used by the pipeline
    GraphicsPipelineBuilder &setPipelineLayout(vk::PipelineLayout &);

    /// Set the renderPass use by the pipeline
    GraphicsPipelineBuilder &setRenderPass(vk::RenderPass &);

    GraphicsPipelineBuilder &setVertexShaderPath(const std::string &);
    GraphicsPipelineBuilder &setFragmentShaderPath(const std::string &);

    GraphicsPipelineBuilder &setMsaaSample(vk::SampleCountFlagBits &);

    GraphicsPipelineBuilder &setPolygonMode(vk::PolygonMode &);
    GraphicsPipelineBuilder &setFaceCulling(vk::CullModeFlags, vk::FrontFace);

    /// Build the Vulkan pipeline
    ///
    /// @return A valid Vulkan pipeline, or VK_NULL_REFERENCE if an error has occurred
    vk::Pipeline build(vk::Device &device, vk::PipelineCache pipelineCache = VK_NULL_HANDLE)const;

public:
    /// @cond
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
    /// @endcond
};

}    // namespace pivot::graphics

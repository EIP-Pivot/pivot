#pragma once

#include "pivot/graphics/interface/IPipelineBuilder.hxx"

#include <optional>
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

    GraphicsPipelineBuilder &setPipelineLayout(vk::PipelineLayout &) noexcept;

    /// Set the renderPass use by the pipeline, can be chained
    GraphicsPipelineBuilder &setRenderPass(vk::RenderPass &) noexcept;

    /// Set the path of the vertex shader to use, can be chained
    GraphicsPipelineBuilder &setVertexShaderPath(const std::string &) noexcept;
    /// Set the path of the fragment shader to use, can be chained
    GraphicsPipelineBuilder &setFragmentShaderPath(const std::string &) noexcept;
    /// Set the path of the geometry shader to use, can be chained
    GraphicsPipelineBuilder &setGeometryShaderPath(const std::string &) noexcept;
    /// Set the path of the tessellation evaluation shader to use, can be chained
    GraphicsPipelineBuilder &setTessellationEvaluationShaderPath(const std::string &) noexcept;
    /// Set the path of the tessellation control shader to use, can be chained
    GraphicsPipelineBuilder &setTessellationControlShaderPath(const std::string &) noexcept;
    /// Set the amount of super sampling, can be chained
    GraphicsPipelineBuilder &setMsaaSample(const vk::SampleCountFlagBits &) noexcept;
    /// Set the rasterizer' polygon mode, can be chained
    GraphicsPipelineBuilder &setPolygonMode(const vk::PolygonMode &) noexcept;
    /// Set the rasterizer' face culling config, can be chained
    GraphicsPipelineBuilder &setFaceCulling(const vk::CullModeFlags &, const vk::FrontFace &) noexcept;

    /// Return the vertex description vector
    std::vector<vk::VertexInputBindingDescription> &getVertexDescription() noexcept;
    /// Return the vertex attributes vector
    std::vector<vk::VertexInputAttributeDescription> &getVertexAttributes() noexcept;

    /// Build the Vulkan pipeline
    ///
    /// @return A valid Vulkan pipeline, or VK_NULL_REFERENCE if an error has occurred
    vk::Pipeline build(vk::Device &device, vk::PipelineCache pipelineCache = VK_NULL_HANDLE) const;

private:
    std::string vertexShaderPath;
    std::optional<std::string> fragmentShaderPath;
    std::optional<std::string> geometryShaderPath;
    std::optional<std::string> tessellationControlShaderPath;
    std::optional<std::string> tessellationEvaluationShaderPath;

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    vk::PipelineMultisampleStateCreateInfo multisampling;
    vk::PipelineDepthStencilStateCreateInfo depthStencil;
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    vk::Viewport viewport;
    vk::Rect2D scissor;
    vk::PipelineLayout pipelineLayout;
    vk::RenderPass renderPass;
    std::vector<vk::VertexInputBindingDescription> vertexDescription;
    std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
};

}    // namespace pivot::graphics

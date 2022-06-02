#pragma once

#include "pivot/graphics/interface/IPipelineBuilder.hxx"
#include "pivot/graphics/vk_init.hxx"

#include <optional>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// @class GraphicsPipelineBuilder
///
/// @brief Utility class to ease the creation of Vulkan pipelines
class GraphicsPipelineBuilder final : public internal::IPipelineBuilder
{
public:
    GraphicsPipelineBuilder(ShaderStorage &shader);
    ~GraphicsPipelineBuilder();

    GraphicsPipelineBuilder &setPipelineLayout(vk::PipelineLayout &layout) noexcept
    {
        pipelineLayout = layout;
        return *this;
    }
    /// Set the renderPass use by the pipeline, can be chained
    GraphicsPipelineBuilder &setRenderPass(vk::RenderPass &pass) noexcept
    {
        renderPass = pass;
        return *this;
    }

    /// Set the path of the vertex shader to use, can be chained
    GraphicsPipelineBuilder &setVertexShaderPath(const std::string &path) noexcept
    {
        vertexShaderPath = path;
        return *this;
    }
    /// Set the path of the fragment shader to use, can be chained
    GraphicsPipelineBuilder &setFragmentShaderPath(const std::string &path) noexcept
    {
        fragmentShaderPath = path;
        return *this;
    }
    /// Set the path of the geometry shader to use, can be chained
    GraphicsPipelineBuilder &setGeometryShaderPath(const std::string &path) noexcept
    {
        geometryShaderPath = path;
        return *this;
    }
    /// Set the path of the tessellation evaluation shader to use, can be chained
    GraphicsPipelineBuilder &setTessellationEvaluationShaderPath(const std::string &path) noexcept
    {
        tessellationEvaluationShaderPath = path;
        return *this;
    }
    /// Set the path of the tessellation control shader to use, can be chained
    GraphicsPipelineBuilder &setTessellationControlShaderPath(const std::string &path) noexcept
    {
        tessellationControlShaderPath = path;
        return *this;
    }
    /// Set the amount of super sampling, can be chained
    GraphicsPipelineBuilder &setMsaaSample(const vk::SampleCountFlagBits &samples) noexcept
    {
        multisampling = vk_init::populateVkPipelineMultisampleStateCreateInfo(samples);
        return *this;
    }
    /// Set the rasterizer' polygon mode, can be chained
    GraphicsPipelineBuilder &setPolygonMode(const vk::PolygonMode &mode) noexcept
    {
        rasterizer.setPolygonMode(mode);
        return *this;
    }
    /// Set the rasterizer' face culling config, can be chained
    GraphicsPipelineBuilder &setFaceCulling(const vk::CullModeFlags &mode, const vk::FrontFace &face) noexcept
    {
        rasterizer.setCullMode(mode);
        rasterizer.setFrontFace(face);
        return *this;
    }
    /// Set the viewport of the pipeline. If none is provided, it will be considered as dynamic state
    GraphicsPipelineBuilder &setViewPort(const vk::Viewport &port)
    {
        viewport = port;
        return *this;
    }
    /// Set the scissor of the pipeline. If none is provided, it will be considered as dynamic state
    GraphicsPipelineBuilder &setScissor(const vk::Rect2D &sc)
    {
        scissor = sc;
        return *this;
    }

    /// Set the Vertex Descriptions for the pipeline
    GraphicsPipelineBuilder &setVertexDescription(const std::vector<vk::VertexInputBindingDescription> &description)
    {
        vertexDescription = description;
        return *this;
    }
    /// Set the Vertex Attributes for the pipeline
    GraphicsPipelineBuilder &setVertexAttributes(const std::vector<vk::VertexInputAttributeDescription> &attributes)
    {
        vertexAttributes = attributes;
        return *this;
    }

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
    std::optional<vk::Viewport> viewport;
    std::optional<vk::Rect2D> scissor;
    vk::PipelineLayout pipelineLayout;
    vk::RenderPass renderPass;
    std::vector<vk::VertexInputBindingDescription> vertexDescription;
    std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
};

}    // namespace pivot::graphics

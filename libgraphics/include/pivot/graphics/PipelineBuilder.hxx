#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// @class GraphicsPipelineBuilder
///
/// @brief Utility class to ease the creation of Vulkan pipelines
/// You must provide and fill all the member variable before calling the build() method
class GraphicsPipelineBuilder
{
public:
    GraphicsPipelineBuilder() = default;
    ~GraphicsPipelineBuilder() = default;

    /// Build the Vulkan pipeline
    ///
    /// @return A valid Vulkan pipeline, or VK_NULL_HANDLE if an error has occurred
    vk::Pipeline build(vk::Device device, vk::RenderPass renderPass, vk::PipelineCache pipelineCache = VK_NULL_HANDLE);

public:
    /// @cond
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    vk::Viewport viewport{};
    vk::Rect2D scissor{};
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    vk::PipelineMultisampleStateCreateInfo multisampling{};
    vk::PipelineLayout pipelineLayout{};
    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    /// @endcond
};

/// @class ComputePipelineBuilder
///
/// @brief Utility class to ease the creation of Vulkan pipelines
/// You must provide and fill all the member variable before calling the build() method
class ComputePipelineBuilder
{
public:
    ComputePipelineBuilder() = default;
    ~ComputePipelineBuilder() = default;

    /// Build the Vulkan pipeline
    ///
    /// @return A valid Vulkan pipeline, or VK_NULL_HANDLE if an error has occurred
    vk::Pipeline build(vk::Device device, vk::PipelineCache pipelineCache = VK_NULL_HANDLE);

public:
    /// @cond
    vk::PipelineShaderStageCreateInfo shaderStage;
    vk::PipelineLayout pipelineLayout;
    /// @endcond
};

}    // namespace pivot::graphics

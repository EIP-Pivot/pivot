#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

class PipelineBuilder
{
public:
    PipelineBuilder() = default;
    ~PipelineBuilder() = default;
    vk::Pipeline build(vk::Device &, vk::RenderPass &);

public:
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    vk::Viewport viewport;
    vk::Rect2D scissor;
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    vk::PipelineMultisampleStateCreateInfo multisampling;
    vk::PipelineLayout pipelineLayout;
    vk::PipelineDepthStencilStateCreateInfo depthStencil;
};

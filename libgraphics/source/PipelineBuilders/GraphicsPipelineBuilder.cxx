#include "pivot/graphics/PipelineBuilders/GraphicsPipelineBuilder.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>

namespace pivot::graphics
{

GraphicsPipelineBuilder::GraphicsPipelineBuilder(const vk::Extent2D &extent)
    : inputAssembly(vk_init::populateVkPipelineInputAssemblyCreateInfo(vk::PrimitiveTopology::eTriangleList, VK_FALSE)),
      colorBlendAttachment(vk_init::populateVkPipelineColorBlendAttachmentState()),
      multisampling(vk_init::populateVkPipelineMultisampleStateCreateInfo(vk::SampleCountFlagBits::e1)),
      depthStencil(vk_init::populateVkPipelineDepthStencilStateCreateInfo()),
      rasterizer(vk_init::populateVkPipelineRasterizationStateCreateInfo(vk::PolygonMode::eFill)),
      viewport(vk::Viewport{
          .x = 0.0f,
          .y = 0.0f,
          .width = static_cast<float>(extent.width),
          .height = static_cast<float>(extent.height),
          .minDepth = 0.0f,
          .maxDepth = 1.0f,
      }),
      scissor(vk::Rect2D{
          .offset = vk::Offset2D{0, 0},
          .extent = extent,
      })
{
}

GraphicsPipelineBuilder::~GraphicsPipelineBuilder() {}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setPipelineLayout(vk::PipelineLayout &layout)
{
    pipelineLayout = layout;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setRenderPass(vk::RenderPass &pass)
{
    renderPass = pass;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setVertexShaderPath(const std::string &p)
{
    vertexShaderPath = p;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setFragmentShaderPath(const std::string &p)
{
    fragmentShaderPath = p;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setMsaaSample(const vk::SampleCountFlagBits &s)
{
    multisampling = vk_init::populateVkPipelineMultisampleStateCreateInfo(s);
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setPolygonMode(const vk::PolygonMode &mode)
{
    rasterizer.setPolygonMode(mode);
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setFaceCulling(const vk::CullModeFlags &mode,
                                                                 const vk::FrontFace &face)
{
    rasterizer.setFrontFace(face);
    rasterizer.setCullMode(mode);
    return *this;
}

vk::Pipeline GraphicsPipelineBuilder::build(vk::Device &device, vk::PipelineCache pipelineCache) const
{
    auto vertShaderCode = vk_utils::readFile(vertexShaderPath);
    auto fragShaderCode = vk_utils::readFile(fragmentShaderPath);

    auto vertShaderModule = vk_utils::createShaderModule(device, vertShaderCode);
    auto fragShaderModule = vk_utils::createShaderModule(device, fragShaderCode);
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{
        vk_init::populateVkPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits::eVertex, vertShaderModule),
        vk_init::populateVkPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits::eFragment, fragShaderModule),
    };

    std::vector<vk::VertexInputBindingDescription> vertexBindings = {Vertex::getBindingDescription()};
    std::vector<vk::VertexInputAttributeDescription> vertexAttributes = Vertex::getAttributeDescriptons();
    auto vertexInputInfo = vk_init::populateVkPipelineVertexInputStateCreateInfo(vertexBindings, vertexAttributes);

    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    vk::GraphicsPipelineCreateInfo pipelineInfo{
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
    };

    vk::Pipeline newPipeline{};
    vk::Result result;
    std::tie(result, newPipeline) = device.createGraphicsPipeline(pipelineCache, pipelineInfo);
    device.destroy(fragShaderModule);
    device.destroy(vertShaderModule);
    vk_utils::vk_try(result);
    return newPipeline;
}

}    // namespace pivot::graphics
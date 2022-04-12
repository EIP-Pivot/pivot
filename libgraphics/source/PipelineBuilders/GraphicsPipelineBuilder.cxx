#include "pivot/graphics/PipelineBuilders/GraphicsPipelineBuilder.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/types/Vertex.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

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
      }),
      vertexDescription({Vertex::getBindingDescription()}),
      vertexAttributes(Vertex::getAttributeDescriptons())
{
}

GraphicsPipelineBuilder::~GraphicsPipelineBuilder() {}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setPipelineLayout(vk::PipelineLayout &layout) noexcept
{
    pipelineLayout = layout;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setRenderPass(vk::RenderPass &pass) noexcept
{
    renderPass = pass;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setVertexShaderPath(const std::string &p) noexcept
{
    vertexShaderPath = p;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setFragmentShaderPath(const std::string &p) noexcept
{
    fragmentShaderPath = p;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setGeometryShaderPath(const std::string &p) noexcept
{
    geometryShaderPath = p;
    return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setTessellationEvaluationShaderPath(const std::string &p) noexcept
{
    tessellationEvaluationShaderPath = p;
    return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setTessellationControlShaderPath(const std::string &p) noexcept
{
    tessellationControlShaderPath = p;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setMsaaSample(const vk::SampleCountFlagBits &s) noexcept
{
    multisampling = vk_init::populateVkPipelineMultisampleStateCreateInfo(s);
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setPolygonMode(const vk::PolygonMode &mode) noexcept
{
    rasterizer.setPolygonMode(mode);
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setFaceCulling(const vk::CullModeFlags &mode,
                                                                 const vk::FrontFace &face) noexcept
{
    rasterizer.setFrontFace(face);
    rasterizer.setCullMode(mode);
    return *this;
}

std::vector<vk::VertexInputBindingDescription> &GraphicsPipelineBuilder::getVertexDescription() noexcept
{
    return vertexDescription;
}
std::vector<vk::VertexInputAttributeDescription> &GraphicsPipelineBuilder::getVertexAttributes() noexcept
{
    return vertexAttributes;
}

static void loadShader(const std::string &path, const vk::ShaderStageFlagBits &stage, vk::Device &device,
                       std::vector<vk::PipelineShaderStageCreateInfo> &shaderStages) noexcept
{
    auto shaderCode = vk_utils::readFile(path);
    auto shaderModule = vk_utils::createShaderModule(device, shaderCode);
    vk_debug::setObjectName(device, shaderModule, path);
    shaderStages.push_back(vk_init::populateVkPipelineShaderStageCreateInfo(stage, shaderModule));
}

vk::Pipeline GraphicsPipelineBuilder::build(vk::Device &device, vk::PipelineCache pipelineCache) const
{
    DEBUG_FUNCTION
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    loadShader(vertexShaderPath, vk::ShaderStageFlagBits::eVertex, device, shaderStages);
    if (fragmentShaderPath)
        loadShader(fragmentShaderPath.value(), vk::ShaderStageFlagBits::eFragment, device, shaderStages);
    if (tessellationControlShaderPath)
        loadShader(tessellationControlShaderPath.value(), vk::ShaderStageFlagBits::eTessellationControl, device,
                   shaderStages);
    if (tessellationEvaluationShaderPath)
        loadShader(tessellationEvaluationShaderPath.value(), vk::ShaderStageFlagBits::eTessellationEvaluation, device,
                   shaderStages);
    if (geometryShaderPath)
        loadShader(geometryShaderPath.value(), vk::ShaderStageFlagBits::eGeometry, device, shaderStages);

    auto vertexInputInfo = vk_init::populateVkPipelineVertexInputStateCreateInfo(vertexDescription, vertexAttributes);

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
    std::for_each(shaderStages.begin(), shaderStages.end(), [&](auto &i) { device.destroyShaderModule(i.module); });
    vk_utils::vk_try(result);
    return newPipeline;
}

}    // namespace pivot::graphics

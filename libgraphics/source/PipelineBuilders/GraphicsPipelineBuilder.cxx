#include "pivot/graphics/PipelineBuilders/GraphicsPipelineBuilder.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/types/Vertex.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics
{

static void loadShader(const VulkanShader &shader, const vk::ShaderStageFlagBits &stage, vk::Device &device,
                       std::vector<vk::PipelineShaderStageCreateInfo> &shaderStages)
{
    auto shaderModule = vk_utils::createShaderModule(device, shader.getByteCode());
    vk_debug::setObjectName(device, shaderModule, shader.getPath(true).string());
    shaderStages.push_back(vk_init::populateVkPipelineShaderStageCreateInfo(stage, shaderModule));
}

GraphicsPipelineBuilder::GraphicsPipelineBuilder(ShaderStorage &shader)
    : internal::IPipelineBuilder(shader),
      inputAssembly(vk_init::populateVkPipelineInputAssemblyCreateInfo(vk::PrimitiveTopology::eTriangleList, VK_FALSE)),
      colorBlendAttachment(vk_init::populateVkPipelineColorBlendAttachmentState()),
      multisampling(vk_init::populateVkPipelineMultisampleStateCreateInfo(vk::SampleCountFlagBits::e1)),
      depthStencil(vk_init::populateVkPipelineDepthStencilStateCreateInfo()),
      rasterizer(vk_init::populateVkPipelineRasterizationStateCreateInfo(vk::PolygonMode::eFill)),
      vertexDescription({Vertex::getBindingDescription()}),
      vertexAttributes(Vertex::getInputAttributeDescriptions(
          0, VertexComponentFlagBits::Position | VertexComponentFlagBits::Normal | VertexComponentFlagBits::UV))
{
}

GraphicsPipelineBuilder::~GraphicsPipelineBuilder() {}

vk::Pipeline GraphicsPipelineBuilder::build(vk::Device &device, vk::PipelineCache pipelineCache) const
{
    DEBUG_FUNCTION
    auto shaderStages = build_shader(device);
    return build_impl(device, shaderStages, pipelineCache);
}

std::vector<vk::PipelineShaderStageCreateInfo> GraphicsPipelineBuilder::build_shader(vk::Device &device) const
{
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    auto vertexId = shader.load(vertexShaderPath);
    loadShader(shader.get(vertexId), vk::ShaderStageFlagBits::eVertex, device, shaderStages);
    if (fragmentShaderPath) {
        auto id = shader.load(fragmentShaderPath.value());
        loadShader(shader.get(id), vk::ShaderStageFlagBits::eFragment, device, shaderStages);
    }
    if (tessellationControlShaderPath) {
        auto id = shader.load(tessellationControlShaderPath.value());
        loadShader(shader.get(id), vk::ShaderStageFlagBits::eTessellationControl, device, shaderStages);
    }
    if (tessellationEvaluationShaderPath) {
        auto id = shader.load(tessellationEvaluationShaderPath.value());
        loadShader(shader.get(id), vk::ShaderStageFlagBits::eTessellationEvaluation, device, shaderStages);
    }
    if (geometryShaderPath) {
        auto id = shader.load(geometryShaderPath.value());
        loadShader(shader.get(id), vk::ShaderStageFlagBits::eGeometry, device, shaderStages);
    }
    return shaderStages;
}

vk::Pipeline GraphicsPipelineBuilder::build_impl(vk::Device &device,
                                                 const std::vector<vk::PipelineShaderStageCreateInfo> &shaderStages,
                                                 vk::PipelineCache pipelineCache) const
{
    auto vertexInputInfo = vk_init::populateVkPipelineVertexInputStateCreateInfo(vertexDescription, vertexAttributes);

    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = 1,
        .pViewports = (viewport.has_value()) ? (&(viewport.value())) : (nullptr),
        .scissorCount = 1,
        .pScissors = (scissor.has_value()) ? (&(scissor.value())) : (nullptr),
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    std::vector<vk::DynamicState> dynamicStateVec;
    if (!viewport.has_value()) dynamicStateVec.push_back(vk::DynamicState::eViewport);
    if (!scissor.has_value()) dynamicStateVec.push_back(vk::DynamicState::eScissor);
    vk::PipelineDynamicStateCreateInfo dynamicState{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStateVec.size()),
        .pDynamicStates = dynamicStateVec.data(),
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
        .pDynamicState = &dynamicState,
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

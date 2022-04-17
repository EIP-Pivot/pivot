#include "pivot/graphics/PipelineStorage.hxx"
#include "pivot/graphics/vk_debug.hxx"

namespace pivot::graphics
{

PipelineStorage::PipelineStorage(VulkanBase &base): base_ref(base) {}

PipelineStorage::~PipelineStorage() {}

void PipelineStorage::init()
{
    vk::PipelineCacheCreateInfo createInfo{};
    pipelineCache = base_ref.device.createPipelineCache(createInfo);
}

void PipelineStorage::destroy()
{

    if (pipelineCache) base_ref.device.destroyPipelineCache(pipelineCache);
    std::ranges::for_each(graphicsStorage, [&](const auto &i) { base_ref.device.destroyPipeline(i.second); });
    std::ranges::for_each(computeStorage, [&](const auto &i) { base_ref.device.destroyPipeline(i.second); });
}

void PipelineStorage::newGraphicsPipeline(const std::string &name, const GraphicsPipelineBuilder &builder)
{
    auto pipeline = builder.build(base_ref.device, pipelineCache);
    if (!pipeline) throw std::runtime_error("Failed to create a pipeline named " + name);
    if (graphicsStorage.contains(name)) {
        logger.warn("Pipeline Storage") << "A graphics pipeline named " + name +
                                               " already exist, replacing with new one";
        removePipeline(name);
    }
    graphicsStorage.insert(std::make_pair(name, pipeline));

    vk_debug::setObjectName(base_ref.device, pipeline, "Graphics pieline : " + name);
}

void PipelineStorage::newComputePipeline(const std::string &name, const ComputePipelineBuilder &builder)
{
    auto pipeline = builder.build(base_ref.device, pipelineCache);
    if (!pipeline) throw std::runtime_error("Failed to create a pipeline named " + name);
    if (computeStorage.contains(name)) {
        logger.warn("Pipeline Builder") << "A compute pipeline named " + name +
                                               " already exist, replacing with new one";
        removePipeline(name);
    }
    computeStorage.insert(std::make_pair(name, pipeline));

    vk_debug::setObjectName(base_ref.device, pipeline, "Compute pieline : " + name);
}

void PipelineStorage::removePipeline(const std::string &name)
{
    auto iter = graphicsStorage.find(name);
    if (iter != graphicsStorage.end()) {
        base_ref.device.destroyPipeline(iter->second);
        graphicsStorage.erase(name);
        return;
    }
    iter = computeStorage.find(name);
    if (iter != computeStorage.end()) {
        base_ref.device.destroyPipeline(iter->second);
        computeStorage.erase(name);
        return;
    }
    throw std::out_of_range("Pipeline not found in storage");
}

}    // namespace pivot::graphics

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
    for (auto &[_, pipeline]: storage) base_ref.device.destroyPipeline(pipeline);
}

void PipelineStorage::newPipeline(const std::string &name, const interface::IPipelineBuilder &builder)
{
    auto pipeline = builder.build(base_ref.device, pipelineCache);
    assert(pipeline);
    storage.insert(std::make_pair(name, pipeline));

    vk_debug::setObjectName(base_ref.device, pipeline, name + " " + builder.getDebugPipelineName());
}

void PipelineStorage::removePipeline(const std::string &name)
{
    base_ref.device.destroyPipeline(get(name));
    storage.erase(name);
}

}    // namespace pivot::graphics
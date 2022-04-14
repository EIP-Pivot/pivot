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

std::vector<std::string> PipelineStorage::getNames() const
{
    std::vector<std::string> names;
    names.reserve(storage.size());
    for (const auto &[name, _]: storage) { names.push_back(name); }
    return names;
}

void PipelineStorage::newPipeline(const std::string &name, const internal::IPipelineBuilder &builder)
{
    auto pipeline = builder.build(base_ref.device, pipelineCache);
    if (!pipeline) throw std::runtime_error("Failed to create a pipeline named " + name);
    storage.insert(std::make_pair(name, pipeline));

    vk_debug::setObjectName(base_ref.device, pipeline, name + " " + builder.getDebugPipelineName());
}

void PipelineStorage::removePipeline(const std::string &name)
{
    base_ref.device.destroyPipeline(get(name));
    storage.erase(name);
}

}    // namespace pivot::graphics

#include "pivot/graphics/ResolverDispatcher.hxx"

#include "pivot/pivot.hxx"

namespace pivot::graphics
{

void ResolverDispatcher::removeResolver(unsigned setId)
{
    DEBUG_FUNCTION
    if (resolverStorage.size() <= setId) { throw std::runtime_error("Set Id is too big"); }
    if (!verifyMsg(resolverStorage[setId] != nullptr, "Set id is not in use !")) { return; }

    if (base_ref.has_value()) { resolverStorage[setId]->destroy(base_ref->get()); }
    resolverStorage[setId] = nullptr;

    unsigned result = std::erase_if(resolverTypes, [setId](const auto &item) { return item.second == setId; });
    verifyMsg(result == 1, "Erased more than one object, that is not possible");
}

std::vector<vk::DescriptorSetLayout> ResolverDispatcher::getDescriptorPair() const
{
    DEBUG_FUNCTION
    std::vector<vk::DescriptorSetLayout> resolver;
    for (auto &item: resolverStorage) {
        if (!item) break;
        resolver.push_back(item->getManagedDescriptorSet().layout);
    }
    return resolver;
}

unsigned ResolverDispatcher::initialize(VulkanBase &base, const AssetStorage &assetStorage, DescriptorBuilder builder)
{
    DEBUG_FUNCTION
    unsigned counter = 0;
    base_ref = base;
    for (auto &item: resolverStorage) {
        if (!item) break;
        DescriptorBuilder copyBuilder = builder;
        item->initialize(base, assetStorage, copyBuilder);
        counter += 1;
    }

    // Make sure no other resolver is left in the end of the array
    verifyMsg(std::find_if(resolverStorage.begin() + counter, resolverStorage.end(),
                           [](const auto &i) { return i != nullptr; }) == resolverStorage.end(),
              "The resolver storage is not contiguous ! Left over resolver will be ignored during rendering");
    return counter;
}

unsigned ResolverDispatcher::destroy(VulkanBase &base)
{
    unsigned counter = 0;

    base_ref = std::nullopt;
    for (auto &item: resolverStorage) {
        if (item) {
            item->destroy(base);
            counter += 1;
        }
    }
    return counter;
}

void ResolverDispatcher::bind(vk::CommandBuffer &cmd, const vk::PipelineLayout &pipelineLayout,
                              vk::PipelineBindPoint bindPoint)
{
    for (unsigned i = 0; i < resolverStorage.size(); i++) {
        std::unique_ptr<IResolver> &resolver = resolverStorage[i];
        if (!resolver) break;

        resolver->bind(cmd);
        const DescriptorPair &descriptor = resolver->getManagedDescriptorSet();
        cmd.bindDescriptorSets(bindPoint, pipelineLayout, i, descriptor.set, nullptr);
    }
}

bool ResolverDispatcher::prepareForDraw(const DrawSceneInformation &information)
{
    for (auto &item: resolverStorage) {
        if (!item) break;
        if (!item->prepareForDraw(information)) { return false; }
    }
    return true;
}

}    // namespace pivot::graphics

#pragma once

#include "pivot/graphics/interface/IResolver.hxx"
#include "pivot/pivot.hxx"

#include <memory>
#include <typeindex>
#include <vector>

namespace pivot::graphics
{

class ResolverDispatcher
{
public:
    constexpr static auto MaxDescriptor = 9;

public:
    template <typename T>
    requires std::is_base_of_v<IResolver, T>
    void addResolver(unsigned setId)
    {
        if (!pivot_check(setId <= resolverStorage.size(), "")) { throw std::runtime_error("Set Id is too big"); }
        if (!pivot_check(resolverStorage[setId] == nullptr, "Set id is already in use !")) { removeResolver(setId); }

        resolverStorage[setId] = std::make_unique<T>();
        resolverTypes.emplace(std::type_index(typeid(T)), setId);
    }

    template <typename T>
    requires std::is_base_of_v<IResolver, T> T &get()
    {
        const std::type_index info(typeid(T));
        auto item = resolverTypes.find(info);

        pivot_assert(item != resolverTypes.end(), "Resolver type " << info.name() << " is unknowned.");
        T *resolver = dynamic_cast<T *>(resolverStorage[item->second].get());
        return *resolver;
    }

    std::vector<vk::DescriptorSetLayout> getDescriptorPair() const;

    void removeResolver(unsigned setId);

    unsigned initialize(VulkanBase &base, const AssetStorage &assetStorage, DescriptorBuilder builder);
    unsigned destroy(VulkanBase &base);

    void bind(vk::CommandBuffer &cmd, const vk::PipelineLayout &pipelineLayout, vk::PipelineBindPoint bindPoint);

    bool prepareForDraw(const DrawSceneInformation &information);

private:
    OptionalRef<VulkanBase> base_ref;

    std::unordered_map<std::type_index, unsigned> resolverTypes;
    std::array<std::unique_ptr<IResolver>, MaxDescriptor> resolverStorage = {nullptr};
};

}    // namespace pivot::graphics

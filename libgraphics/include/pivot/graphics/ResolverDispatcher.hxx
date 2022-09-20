#pragma once

#include "pivot/graphics/interface/IResolver.hxx"
#include "pivot/pivot.hxx"

#include <memory>
#include <typeindex>
#include <vector>

namespace pivot::graphics
{

/// Manage all of the dispatcher of the frame
class ResolverDispatcher
{
public:
    /// Max amount of descriptor set
    constexpr static auto MaxDescriptor = 9;

public:
    template <typename T>
    /// Add a new resolver responsible for descriptor set number setId
    requires std::is_base_of_v<IResolver, T>
    void addResolver(unsigned setId)
    {
        if (!pivot_check(setId <= resolverStorage.size(), "")) { throw std::runtime_error("Set Id is too big"); }
        if (!pivot_check(resolverStorage[setId] == nullptr, "Set id is already in use !")) { removeResolver(setId); }

        resolverStorage[setId] = std::make_unique<T>();
        resolverTypes.emplace(std::type_index(typeid(T)), setId);
    }

    template <typename T>
    /// Return a specific Resolver
    requires std::is_base_of_v<IResolver, T> T &get()
    {
        const std::type_index info(typeid(T));
        auto item = resolverTypes.find(info);
        pivot_assert(item != resolverTypes.end(), "Resolver type " << info.name() << " is unknowned.");

        T *resolver = dynamic_cast<T *>(resolverStorage[item->second].get());
        pivot_assert(resolver, "Resolver type " << info.name() << " failed to be downcasted.");
        return *resolver;
    }

    /// Return all of descriptor set layout managed by the dispatcher
    std::vector<vk::DescriptorSetLayout> getDescriptorPair() const;

    /// Remove resolver from the dispatcher
    void removeResolver(unsigned setId);

    /// Initialize all resolver in the dispatcher
    unsigned initialize(VulkanBase &base, const AssetStorage &assetStorage, DescriptorBuilder builder);
    /// Destroy all resolver in the dispatcher
    unsigned destroy(VulkanBase &base);

    /// Bind the resolvers into the command buffer
    void bind(vk::CommandBuffer &cmd, const vk::PipelineLayout &pipelineLayout, vk::PipelineBindPoint bindPoint);

    /// Prepare the resolvers for rendering the scene
    bool prepareForDraw(const DrawSceneInformation &information);

private:
    OptionalRef<VulkanBase> base_ref;

    std::unordered_map<std::type_index, unsigned> resolverTypes;
    std::array<std::unique_ptr<IResolver>, MaxDescriptor> resolverStorage = {nullptr};
};

}    // namespace pivot::graphics

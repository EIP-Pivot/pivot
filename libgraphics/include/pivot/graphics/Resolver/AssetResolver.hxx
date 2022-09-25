#pragma once

#include "pivot/graphics/interface/IResolver.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

namespace pivot::graphics
{

/// Manage the assets in the scene
class AssetResolver : public IResolver
{
public:
    using IResolver::IResolver;

    /// Initialize the ressources
    bool initialize(VulkanBase &, const AssetStorage &, DescriptorBuilder &) override;
    /// Destroy them
    bool destroy(VulkanBase &base) override;

    /// Build the buffer for the draw
    bool prepareForDraw(const DrawSceneInformation &sceneInformation) override;

    DescriptorPair getManagedDescriptorSet() const override
    {
        return {
            .layout = storage_ref->get().getDescriptorSetLayout(),
            .set = storage_ref->get().getDescriptorSet(),
        };
    }

    void bind(vk::CommandBuffer &cmd) override;

private:
    OptionalRef<VulkanBase> base_ref;
    OptionalRef<const AssetStorage> storage_ref;
    vk::DescriptorSet descriptorSet;
};

}    // namespace pivot::graphics

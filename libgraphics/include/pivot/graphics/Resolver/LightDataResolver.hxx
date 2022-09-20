#pragma once

#include "pivot/graphics/interface/IResolver.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

namespace pivot::graphics
{

/// Managed the Light in the scene
class LightDataResolver : public IResolver
{
private:
    struct Frame {
        /// The number of pointlight in the buffers
        vk::DeviceSize pointLightCount = 0;
        /// The number of directional in the buffers
        vk::DeviceSize directionalLightCount = 0;
        /// The number of spotlight in the buffers
        vk::DeviceSize spotLightCount = 0;
        /// Hold the omnidirectional light buffer
        AllocatedBuffer<gpu_object::PointLight> omniLightBuffer;
        /// Hold the directional light buffer
        AllocatedBuffer<gpu_object::DirectionalLight> directLightBuffer;
        /// Hold the spot light buffer
        AllocatedBuffer<gpu_object::SpotLight> spotLightBuffer;
    };

private:
    static vk::DescriptorSetLayout descriptorSetLayout;

public:
    /// @return Get the descritor set layout
    static constexpr const vk::DescriptorSetLayout &getDescriptorSetLayout() noexcept { return descriptorSetLayout; }

    /// Initialize the ressources
    bool initialize(VulkanBase &, const AssetStorage &, DescriptorBuilder &) override;
    /// Destroy them
    bool destroy(VulkanBase &base) override;

    /// Build the buffer for the draw
    bool prepareForDraw(const DrawSceneInformation &sceneInformation) override;

    DescriptorPair getManagedDescriptorSet() const override
    {
        return {
            .layout = descriptorSetLayout,
            .set = descriptorSet,
        };
    }

    /// Return the internal frame data
    constexpr const Frame &getFrameData() const noexcept { return frame; }

private:
    void createLightBuffer(std::size_t size = 1);
    void updateDescriptorSet();

private:
    OptionalRef<VulkanBase> base_ref;
    OptionalRef<const AssetStorage> storage_ref;
    vk::DescriptorSet descriptorSet;

    Frame frame;
};

}    // namespace pivot::graphics

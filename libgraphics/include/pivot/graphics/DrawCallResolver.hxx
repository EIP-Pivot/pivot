#pragma once

#include "pivot/graphics/interface/IResolver.hxx"

#include "pivot/graphics/types/vk_types.hxx"
#include "pivot/graphics/vk_debug.hxx"

#include <array>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// @class DrawCallResolver
/// @brief Used to build the per frame draw informations
class DrawCallResolver : public IResolver
{
public:
    /// The default size of the buffers
    static constexpr const auto defaultBufferSize = 42;

    /// Represent a draw batch
    struct DrawBatch {
        /// The id of the mesh
        std::string meshId;
        /// The first index of the batch
        std::uint32_t first;
        /// The size of the batch
        std::uint32_t count;
    };

    /// Represent the index of the draw batch, ordered by pipeline
    struct PipelineBatch {
        /// The id of the pipeline
        std::string pipelineID;
        /// The first index of the batch
        std::uint32_t first;
        /// The size of the batch
        std::uint32_t size;
    };

    /// Represent a frame ressources
    struct Frame {
        /// Hold the indirect command
        AllocatedBuffer<vk::DrawIndexedIndirectCommand> indirectBuffer;
        /// Hold the uniform object buffer
        AllocatedBuffer<gpu_object::UniformBufferObject> objectBuffer;
        /// Hold the omnidirectional light buffer
        AllocatedBuffer<gpu_object::PointLight> omniLightBuffer;
        /// Hold the directional light buffer
        AllocatedBuffer<gpu_object::DirectionalLight> directLightBuffer;
        /// Hold the spot light buffer
        AllocatedBuffer<gpu_object::SpotLight> spotLightBuffer;
        /// The descriptor set holding the object buffer
        vk::DescriptorSet objectDescriptor = VK_NULL_HANDLE;
        /// The draw batches
        std::vector<DrawBatch> packedDraws;
        /// The pipeline batch
        std::vector<PipelineBatch> pipelineBatch;
        /// The current size of the buffer
        vk::DeviceSize currentBufferSize = defaultBufferSize;
        /// The current size of the buffer that is exposed through the descriptor set.
        vk::DeviceSize currentDescriptorSetSize = defaultBufferSize;
        /// The number of pointlight in the buffers
        vk::DeviceSize pointLightCount = 0;
        /// The number of directional in the buffers
        vk::DeviceSize directionalLightCount = 0;
        /// The number of spotlight in the buffers
        vk::DeviceSize spotLightCount = 0;
    };

private:
    static vk::DescriptorSetLayout descriptorSetLayout;

public:
    /// @return Get the descritor set layout
    static constexpr const vk::DescriptorSetLayout &getDescriptorSetLayout() noexcept { return descriptorSetLayout; }

public:
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
            .set = frame.objectDescriptor,
        };
    }

    /// Get the frame data of a given frame
    constexpr const Frame &getFrameData() const { return frame; }

private:
    template <class T, class G>
    requires std::is_constructible_v<G, const T &, const Transform &> && BufferValid<G> vk::DeviceSize
    handleLights(AllocatedBuffer<G> &buffer, const Object<T> &lights, const Object<Transform> &transforms,
                 const std::string &debug_name = "")
    {
        pivot_assert(lights.objects.get().size() == lights.exist.get().size(), "Light ECS data are incorrect");
        pivot_assert(transforms.objects.get().size() == transforms.exist.get().size(),
                     "Transform ECS data are incorrect");

        std::vector<G> lightsData;
        for (unsigned i = 0; i < lights.objects.get().size() && i < transforms.objects.get().size(); i++) {
            if (!lights.exist.get().at(i) || !lights.exist.get().at(i)) continue;
            const auto &light = lights.objects.get().at(i);
            const auto &transform = transforms.objects.get().at(i);

            lightsData.emplace_back(light, transform);
        }
        if (buffer.getSize() < lightsData.size()) {
            if (buffer) base_ref->get().allocator.destroyBuffer(buffer);
            buffer = base_ref->get().allocator.createMappedBuffer<G>(lightsData.size(), debug_name);
        }
        base_ref->get().allocator.copyBuffer(buffer, std::span(lightsData));
        return lightsData.size();
    }

    void createBuffer(vk::DeviceSize bufferSize);
    void createLightBuffer();
    void updateDescriptorSet(const vk::DeviceSize bufferSize);

private:
    OptionalRef<VulkanBase> base_ref;
    OptionalRef<const AssetStorage> storage_ref;
    Frame frame;
};

}    // namespace pivot::graphics

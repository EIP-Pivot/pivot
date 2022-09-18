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
        /// The descriptor set holding the object buffer
        vk::DescriptorSet objectDescriptor = VK_NULL_HANDLE;
        /// The draw batches
        std::vector<DrawBatch> packedDraws;
        /// The pipeline batch
        std::vector<PipelineBatch> pipelineBatch;
        /// The current size of the buffer
        vk::DeviceSize currentBufferSize = defaultBufferSize;
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
    void createBuffer(vk::DeviceSize bufferSize);
    void updateDescriptorSet();

private:
    OptionalRef<VulkanBase> base_ref;
    OptionalRef<const AssetStorage> storage_ref;
    Frame frame;
};

}    // namespace pivot::graphics

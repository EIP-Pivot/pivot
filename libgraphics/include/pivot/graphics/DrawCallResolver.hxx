#pragma once

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/RenderObject.hxx"
#include "pivot/graphics/types/common.hxx"
#include "pivot/graphics/types/vk_types.hxx"

#include <array>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// @class DrawCallResolver
/// @brief Used to build the per frame draw informations
class DrawCallResolver
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
        AllocatedBuffer indirectBuffer{};
        /// Hold the uniform object buffer
        AllocatedBuffer objectBuffer{};
        /// The descriptor set holding the object buffer
        vk::DescriptorSet objectDescriptor = VK_NULL_HANDLE;
        /// The draw batches
        std::vector<DrawBatch> packedDraws;
        /// The pipeline batch
        std::vector<PipelineBatch> pipelineBatch;
        /// The current size of the buffer
        uint32_t currentBufferSize = defaultBufferSize;
        /// The current size of the buffer that is exposed through the descriptor set.
        uint32_t currentDescriptorSetSize = defaultBufferSize;
    };

public:
    /// Constructor
    DrawCallResolver(VulkanBase &, AssetStorage &);
    /// Destructor
    ~DrawCallResolver();

    /// Initialize the ressources
    void init();
    /// Destroy them
    void destroy();

    /// Build the buffer for the draw
    void prepareForDraw(std::vector<std::reference_wrapper<const RenderObject>> &sceneInformation,
                        const uint32_t frameIndex);

    /// Get the frame data of a given frame
    constexpr const Frame &getFrameData(const uint32_t &frameIndex) const { return frames.at(frameIndex); }

    /// @return Get the descritor set layout
    constexpr const vk::DescriptorSetLayout &getDescriptorSetLayout() const noexcept { return descriptorSetLayout; }

private:
    void createDescriptorPool();
    void createBuffers(Frame &frame, const auto bufferSize);
    void createDescriptorSets(Frame &frame, const auto bufferSize);
    void createDescriptorSetLayout();

private:
    OptionalRef<VulkanBase> base_ref;
    OptionalRef<AssetStorage> storage_ref;
    std::array<Frame, MaxFrameInFlight> frames;
    vk::DescriptorPool descriptorPool = VK_NULL_HANDLE;
    vk::DescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
};

}    // namespace pivot::graphics
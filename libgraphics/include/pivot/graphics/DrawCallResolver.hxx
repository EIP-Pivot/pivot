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

class DrawCallResolver
{
public:
    static constexpr const auto defaultBufferSize = 20;
    struct DrawBatch {
        std::string meshId;
        uint32_t first;
        uint32_t count;
    };
    struct Frame {
        AllocatedBuffer indirectBuffer{};
        AllocatedBuffer objectBuffer{};
        vk::DescriptorSet objectDescriptor = VK_NULL_HANDLE;
        std::vector<DrawBatch> packedDraws;
        uint32_t currentBufferSize = defaultBufferSize;
    };

public:
    DrawCallResolver(VulkanBase &, AssetStorage &);
    ~DrawCallResolver();

    void init();
    void destroy();

    void prepareForDraw(const std::vector<std::reference_wrapper<const RenderObject>> &sceneInformation,
                        const gpuObject::CameraData &camera, const uint32_t frameIndex);

    constexpr const Frame &getFrameData(const uint32_t &frameIndex) const { return frames.at(frameIndex); }

private:
    void recreateBuffers(const auto newSize, const std::uint32_t frameIndex);
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
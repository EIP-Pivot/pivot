#pragma once

#include "pivot/graphics/interface/IRenderer.hxx"

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/PipelineStorage.hxx"
#include "pivot/graphics/types/common.hxx"

namespace pivot::graphics
{

class CullingRenderer : public IComputeRenderer
{
public:
    CullingRenderer(PipelineStorage &storage, AssetStorage &assets);
    ~CullingRenderer();

    bool onInit(VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout) override;
    void onStop(VulkanBase &base_ref) override;

    bool onDraw(const CameraData &cameraData, DrawCallResolver &resolver, vk::CommandBuffer &cmd) override;

private:
    void createPipelineLayout(vk::Device &device, vk::DescriptorSetLayout &resolverLayout);
    void createPipeline(vk::Device &device);

private:
    PipelineStorage &stor;
    AssetStorage &assets;
    QueueFamilyIndices indices;

    vk::PipelineLayout cullingLayout = VK_NULL_HANDLE;
};

}    // namespace pivot::graphics
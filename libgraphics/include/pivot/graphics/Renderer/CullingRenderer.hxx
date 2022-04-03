#pragma once

#include "pivot/graphics/interface/IRenderer.hxx"

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/PipelineStorage.hxx"
#include "pivot/graphics/types/common.hxx"

namespace pivot::graphics
{

/// Dispach compute shader to cull the scene
class CullingRenderer : public IComputeRenderer
{
public:
    /// @copydoc IRenderer::IRenderer
    CullingRenderer(PipelineStorage &storage, AssetStorage &assets);
    ~CullingRenderer();

    std::string getName() const noexcept override { return "Culling"; }
    bool onInit(VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout) override;
    void onStop(VulkanBase &base_ref) override;

    bool onDraw(const CameraData &cameraData, DrawCallResolver &resolver, vk::CommandBuffer &cmd) override;

private:
    void createPipelineLayout(vk::Device &device, vk::DescriptorSetLayout &resolverLayout);
    void createPipeline(vk::Device &device);

private:
    QueueFamilyIndices indices;

    vk::PipelineLayout cullingLayout = VK_NULL_HANDLE;
};

}    // namespace pivot::graphics

#pragma once

#include "pivot/graphics/interface/IRenderer.hxx"

#include "pivot/graphics/AssetStorage/AssetStorage.hxx"
#include "pivot/graphics/PipelineStorage.hxx"

namespace pivot::graphics
{

/// Dispach compute shader to cull the scene
class CullingRenderer : public IComputeRenderer
{
public:
    /// @copydoc IRenderer::IRenderer
    CullingRenderer(StorageUtils &utils);
    ~CullingRenderer();

    std::string getName() const noexcept override { return "Culling"; }
    bool onInit(VulkanBase &base_ref, const ResolverDispatcher &dispatcher) override;
    void onStop(VulkanBase &base_ref) override;
    bool onDraw(const RenderingContext &context, const CameraData &cameraData, ResolverDispatcher &dispatcher,
                vk::CommandBuffer &cmd) override;

private:
    void createPipelineLayout(vk::Device &device, const ResolverDispatcher &dispatcher);
    void createPipeline();

private:
    QueueFamilyIndices indices;

    vk::PipelineLayout cullingLayout = VK_NULL_HANDLE;
};

}    // namespace pivot::graphics

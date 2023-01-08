#pragma once

#include "pivot/graphics/interface/IRenderer.hxx"

namespace pivot::graphics
{

/// Draw the scene
class GraphicsRenderer : public IGraphicsRenderer
{
public:
    /// @copydoc IRenderer::IRenderer
    GraphicsRenderer(StorageUtils &utils);
    ~GraphicsRenderer();

    std::string getName() const noexcept override { return "Graphics"; }
    bool onInit(const vk::Extent2D &size, VulkanBase &base_ref, const ResolverDispatcher &dispatcher,
                vk::RenderPass &pass) override;
    void onStop(VulkanBase &base_ref) override;

    bool onDraw(const RenderingContext &context, const CameraData &cameraData, ResolverDispatcher &dispatcher,
                vk::CommandBuffer &cmd) override;

private:
    void createPipelineLayout(vk::Device &device, const ResolverDispatcher &dispatcher);
    void createPipeline(VulkanBase &base_ref, vk::RenderPass &pass);

private:
    bool bIsMultiDraw = false;
    vk::PipelineLayout pipelineLayout = VK_NULL_HANDLE;
};
}    // namespace pivot::graphics

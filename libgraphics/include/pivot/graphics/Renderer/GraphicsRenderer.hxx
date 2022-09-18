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
    bool onInit(const vk::Extent2D &size, VulkanBase &base_ref, const vk::DescriptorSetLayout &resolverLayout,
                const vk::DescriptorSetLayout &lightLayout, vk::RenderPass &pass) override;
    void onStop(VulkanBase &base_ref) override;
    bool onRecreate(const vk::Extent2D &size, VulkanBase &base_ref, const vk::DescriptorSetLayout &resolverLayout,
                    const vk::DescriptorSetLayout &lightLayout, vk::RenderPass &pass) override;
    bool onDraw(const RenderingContext &context, const CameraData &cameraData, DrawCallResolver &resolver,
                LightDataResolver &light, vk::CommandBuffer &cmd) override;

private:
    void createPipelineLayout(vk::Device &device, const vk::DescriptorSetLayout &resolverLayout,
                              const vk::DescriptorSetLayout &lightLayout);
    void createPipeline(VulkanBase &base_ref, vk::RenderPass &pass);

private:
    bool bIsMultiDraw = false;
    vk::PipelineLayout pipelineLayout = VK_NULL_HANDLE;
};
}    // namespace pivot::graphics

#pragma once

#include "pivot/graphics/interface/IRenderer.hxx"

namespace pivot::graphics
{

/// Render ImGui interface
class ImGuiRenderer : public IGraphicsRenderer
{
public:
    /// @copydoc IRenderer::IRenderer
    ImGuiRenderer(StorageUtils &utils);
    ~ImGuiRenderer();

    std::string getName() const noexcept override { return "ImGui"; }
    bool onInit(const vk::Extent2D &size, VulkanBase &base_ref, const vk::DescriptorSetLayout &resolverLayout,
                const vk::DescriptorSetLayout &lightLayout, vk::RenderPass &pass) override;
    void onStop(VulkanBase &base_ref) override;
    bool onRecreate(const vk::Extent2D &size, VulkanBase &base_ref, const vk::DescriptorSetLayout &resolverLayout,
                    const vk::DescriptorSetLayout &lightLayout, vk::RenderPass &pass) override;
    bool onDraw(const RenderingContext &context, const CameraData &cameraData, DrawCallResolver &resolver,
                LightDataResolver &light, vk::CommandBuffer &cmd) override;

private:
    void createImGuiContext(VulkanBase &base_ref, vk::RenderPass &pass);
    void createDescriptorPool(vk::Device &device);

private:
    vk::DescriptorPool pool = VK_NULL_HANDLE;
};

}    // namespace pivot::graphics

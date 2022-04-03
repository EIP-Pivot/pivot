#pragma once

#include "pivot/graphics/interface/IRenderer.hxx"

namespace pivot::graphics
{

class ImGuiRenderer : public IGraphicsRenderer
{
public:
    ImGuiRenderer(PipelineStorage &storage, AssetStorage &assets);
    ~ImGuiRenderer();

    bool onInit(const vk::Extent2D &size, VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout,
                vk::RenderPass &pass) override;
    void onStop(VulkanBase &base_ref) override;
    bool onRecreate(const vk::Extent2D &size, VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout,
                    vk::RenderPass &pass) override;
    bool onDraw(const CameraData &cameraData, DrawCallResolver &resolver, vk::CommandBuffer &cmd) override;

private:
    void createImGuiContext(VulkanBase &base_ref, vk::RenderPass &pass);
    void createDescriptorPool(vk::Device &device);

private:
    vk::DescriptorPool pool = VK_NULL_HANDLE;
};

}    // namespace pivot::graphics

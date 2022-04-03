#pragma once

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/DrawCallResolver.hxx"
#include "pivot/graphics/PipelineStorage.hxx"
#include "pivot/graphics/VulkanBase.hxx"

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// Root interface for the Renderers
class IRenderer
{
public:
    /// Constructor
    IRenderer(PipelineStorage &storage, AssetStorage &assets): stor(storage), assets(assets) {}
    /// Destructor
    virtual ~IRenderer() {}
    /// Return the debug name of the renderer
    virtual std::string getName() const noexcept = 0;
    /// Return the debug type of the renderer
    virtual std::string getType() const noexcept = 0;
    /// Called when the renderer is being removed from the frame
    virtual void onStop(VulkanBase &base_ref) = 0;
    /// Called once per frame
    virtual bool onDraw(const CameraData &cameraData, DrawCallResolver &resolver, vk::CommandBuffer &cmd) = 0;

protected:
    /// Reference to the PipelineStorage
    PipelineStorage &stor;
    /// Reference to the AssetStorage
    AssetStorage &assets;
};

/// Renderer interface for compute-based operation
class IComputeRenderer : public IRenderer
{
public:
    using IRenderer::IRenderer;
    virtual ~IComputeRenderer() {}
    std::string getType() const noexcept final { return "Compute Renderer"; }
    /// Called when the renderer are added to the frame
    virtual bool onInit(VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout) = 0;
};

/// Renderer interface for graphics operation
class IGraphicsRenderer : public IRenderer
{
public:
    using IRenderer::IRenderer;
    virtual ~IGraphicsRenderer() {}
    std::string getType() const noexcept final { return "Graphics Renderer"; }
    /// Called when the renderer are added to the frame
    virtual bool onInit(const vk::Extent2D &size, VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout,
                        vk::RenderPass &pass) = 0;
    /// Called when the target change (ie: swapchain resize, etc...)
    virtual bool onRecreate(const vk::Extent2D &size, VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout,
                            vk::RenderPass &pass) = 0;
};

template <typename T>
/// This concept is valid for a renderer and is able to be constructed
concept validRenderer = requires
{
    std::is_base_of_v<IRenderer, T>;
    std::is_constructible_v<T, PipelineStorage, AssetStorage>;
};

}    // namespace pivot::graphics

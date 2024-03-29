#pragma once

#include "pivot/graphics/AssetStorage/AssetStorage.hxx"
#include "pivot/graphics/PipelineStorage.hxx"
#include "pivot/graphics/ResolverDispatcher.hxx"
#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/types/vk_types.hxx"

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// Wrapper for all the "storage class"
struct StorageUtils {
    /// @see PipelineStorage
    std::reference_wrapper<PipelineStorage> pipeline;
    /// @see AssetStorage
    std::reference_wrapper<AssetStorage> assets;
};

/// Convey information about the rendering
struct RenderingContext {
    /// The rectangle where the rendering should take place
    vk::Rect2D renderArea;
    /// The size of the viewport.
    vk::Extent2D viewport;
};

/// Root interface for the Renderers
class IRenderer
{
public:
    /// Constructor
    IRenderer(StorageUtils &utils): storage(utils) {}
    /// Destructor
    virtual ~IRenderer() {}
    /// Return the debug name of the renderer
    virtual std::string getName() const noexcept = 0;
    /// Return the debug type of the renderer
    virtual std::string getType() const noexcept = 0;
    /// Called when the renderer is being removed from the frame
    virtual void onStop(VulkanBase &base_ref) = 0;
    /// Called once per frame
    virtual bool onDraw(const RenderingContext &context, const CameraData &cameraData, ResolverDispatcher &dispatcher,
                        vk::CommandBuffer &cmd) = 0;

protected:
    /// All ref to the storage class
    StorageUtils storage;
};

/// Renderer interface for compute-based operation
class IComputeRenderer : public IRenderer
{
public:
    using IRenderer::IRenderer;
    virtual ~IComputeRenderer() {}
    std::string getType() const noexcept final { return "Compute Renderer"; }
    /// Called when the renderer are added to the frame
    virtual bool onInit(VulkanBase &base_ref, const ResolverDispatcher &dispatcher) = 0;
};

/// Renderer interface for graphics operation
class IGraphicsRenderer : public IRenderer
{
public:
    using IRenderer::IRenderer;
    virtual ~IGraphicsRenderer() {}
    std::string getType() const noexcept final { return "Graphics Renderer"; }
    /// Called when the renderer are added to the frame
    virtual bool onInit(const vk::Extent2D &size, VulkanBase &base_ref, const ResolverDispatcher &dispatcher,
                        vk::RenderPass &pass) = 0;
};

template <typename T>
/// This concept is valid for a renderer and is able to be constructed
concept validRenderer = std::is_base_of_v<IRenderer, T> && std::is_constructible_v<T, StorageUtils &>;

}    // namespace pivot::graphics

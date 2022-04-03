#pragma once

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/DrawCallResolver.hxx"
#include "pivot/graphics/PipelineStorage.hxx"
#include "pivot/graphics/VulkanBase.hxx"

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

class IRenderer
{
public:
    IRenderer(PipelineStorage &storage, AssetStorage &assets): stor(storage), assets(assets) {}
    virtual ~IRenderer() {}
    virtual void onStop(VulkanBase &base_ref) = 0;
    virtual bool onDraw(const CameraData &cameraData, DrawCallResolver &resolver, vk::CommandBuffer &cmd) = 0;

protected:
    PipelineStorage &stor;
    AssetStorage &assets;
};

class IComputeRenderer : public IRenderer
{
public:
    using IRenderer::IRenderer;
    virtual ~IComputeRenderer() {}
    virtual bool onInit(VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout) = 0;
};

class IGraphicsRenderer : public IRenderer
{
public:
    using IRenderer::IRenderer;
    virtual ~IGraphicsRenderer() {}
    virtual bool onInit(const vk::Extent2D &size, VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout,
                        vk::RenderPass &pass) = 0;
    virtual bool onRecreate(const vk::Extent2D &size, VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout,
                            vk::RenderPass &pass) = 0;
};

template <typename T>
concept validRenderer = requires
{
    std::is_base_of_v<IRenderer, T>;
    std::is_constructible_v<T, PipelineStorage, AssetStorage>;
};

}    // namespace pivot::graphics

#pragma once

#include "pivot/graphics/DrawCallResolver.hxx"
#include "pivot/graphics/PipelineStorage.hxx"
#include "pivot/graphics/VulkanBase.hxx"

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

class IRenderer
{
public:
    virtual ~IRenderer() {}
    virtual void onStop(VulkanBase &base_ref) = 0;
    virtual bool onDraw(const CameraData &cameraData, DrawCallResolver &resolver, vk::CommandBuffer &cmd) = 0;
};

class IComputeRenderer : public IRenderer
{
public:
    virtual ~IComputeRenderer() {}
    virtual bool onInit(VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout) = 0;
};

class IGraphicsRenderer : public IRenderer
{
public:
    virtual ~IGraphicsRenderer() {}
    virtual bool onInit(const vk::Extent2D &size, VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout,
                        vk::RenderPass &pass) = 0;
    virtual bool onRecreate(const vk::Extent2D &size, VulkanBase &base_ref, vk::DescriptorSetLayout &resolverLayout,
                            vk::RenderPass &pass) = 0;
};

}    // namespace pivot::graphics

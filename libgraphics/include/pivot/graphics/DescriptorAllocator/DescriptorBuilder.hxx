#pragma once

#include "pivot/graphics/DescriptorAllocator/DescriptorAllocator.hxx"
#include "pivot/graphics/DescriptorAllocator/DescriptorLayoutCache.hxx"

#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// Helper class to help building a new descriptor set
class DescriptorBuilder
{
public:
    /// Ctor
    DescriptorBuilder(DescriptorLayoutCache &cache, DescriptorAllocator &alloc);
    /// Dtor
    ~DescriptorBuilder();

    /// Add a buffer to the layout.
    DescriptorBuilder &bindBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo, vk::DescriptorType type,
                                  vk::ShaderStageFlags stageFlags);
    /// Add an image to the layout
    DescriptorBuilder &bindImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo, vk::DescriptorType type,
                                 vk::ShaderStageFlags stageFlags);

    /// Build the descriptor set. Return true on success.
    bool build(vk::Device &device, vk::DescriptorSet &set, vk::DescriptorSetLayout &layout);
    /// @copydoc build
    bool build(vk::Device &device, vk::DescriptorSet &set);

private:
    std::vector<vk::WriteDescriptorSet> writes;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    DescriptorLayoutCache &cache;
    DescriptorAllocator &alloc;
};

}    // namespace pivot::graphics

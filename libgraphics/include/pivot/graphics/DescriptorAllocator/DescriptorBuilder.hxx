#pragma once

#include "pivot/graphics/DescriptorAllocator/DescriptorAllocator.hxx"
#include "pivot/graphics/DescriptorAllocator/DescriptorLayoutCache.hxx"

#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

class DescriptorBuilder
{
public:
    DescriptorBuilder(DescriptorLayoutCache &cache, DescriptorAllocator &alloc);
    ~DescriptorBuilder();

    DescriptorBuilder &bindBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo, vk::DescriptorType type,
                                  vk::ShaderStageFlags stageFlags);
    DescriptorBuilder &bindImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo, vk::DescriptorType type,
                                 vk::ShaderStageFlags stageFlags);

    bool build(vk::Device &device, vk::DescriptorSet &set, vk::DescriptorSetLayout &layout);
    bool build(vk::Device &device, vk::DescriptorSet &set);

private:
    std::vector<vk::WriteDescriptorSet> writes;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    DescriptorLayoutCache &cache;
    DescriptorAllocator &alloc;
};

}    // namespace pivot::graphics

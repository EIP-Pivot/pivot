#include "pivot/graphics/DescriptorAllocator/DescriptorBuilder.hxx"

#include "Logger.hpp"

namespace pivot::graphics
{

DescriptorBuilder::DescriptorBuilder(DescriptorLayoutCache &layoutCache, DescriptorAllocator &allocator)
    : cache(layoutCache), alloc(allocator)
{
}

DescriptorBuilder::~DescriptorBuilder() {}

DescriptorBuilder &DescriptorBuilder::bindBuffer(uint32_t binding, vk::DescriptorBufferInfo &bufferInfo,
                                                 vk::DescriptorType type, vk::ShaderStageFlags stageFlags)
{
    vk::DescriptorSetLayoutBinding newBinding{
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = 1,
        .stageFlags = stageFlags,
    };
    bindings.push_back(newBinding);

    vk::WriteDescriptorSet newWrite{
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &bufferInfo,
    };
    writes.push_back(newWrite);
    descriptorBindingFlags.push_back({});
    return *this;
}

DescriptorBuilder &DescriptorBuilder::bindImage(uint32_t binding, vk::DescriptorImageInfo &imageInfo,
                                                vk::DescriptorType type, vk::ShaderStageFlags stageFlags)
{
    vk::DescriptorSetLayoutBinding newBinding{
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = 1,
        .stageFlags = stageFlags,
    };
    bindings.push_back(newBinding);

    vk::WriteDescriptorSet newWrite{
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = &imageInfo,
    };
    writes.push_back(newWrite);
    descriptorBindingFlags.push_back({});
    return *this;
}

DescriptorBuilder &DescriptorBuilder::bindImages(uint32_t binding, std::vector<vk::DescriptorImageInfo> &imageInfo,
                                                 vk::DescriptorType type, vk::ShaderStageFlags stageFlags)
{
    vk::DescriptorSetLayoutBinding newBinding{
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = static_cast<uint32_t>(imageInfo.size()),
        .stageFlags = stageFlags,
    };
    bindings.push_back(newBinding);

    vk::WriteDescriptorSet newWrite{
        .dstBinding = binding,
        .descriptorCount = static_cast<uint32_t>(imageInfo.size()),
        .descriptorType = type,
        .pImageInfo = imageInfo.data(),
    };
    writes.push_back(newWrite);
    variableDescriptorCount.push_back(imageInfo.size());
    descriptorBindingFlags.push_back(vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
                                     vk::DescriptorBindingFlagBits::ePartiallyBound);
    return *this;
}

bool DescriptorBuilder::build(vk::Device &device, vk::DescriptorSet &set, vk::DescriptorSetLayout &layout)
{
    vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingInfo{
        .bindingCount = static_cast<uint32_t>(descriptorBindingFlags.size()),
        .pBindingFlags = descriptorBindingFlags.data(),
    };
    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .pNext = &bindingInfo,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };
    layout = cache.createDescriptorLayout(layoutInfo);

    vk::DescriptorSetVariableDescriptorCountAllocateInfo set_counts{
        .descriptorSetCount = static_cast<uint32_t>(variableDescriptorCount.size()),
        .pDescriptorCounts = variableDescriptorCount.data(),
    };
    if (!alloc.allocate(set, layout, set_counts)) return false;
    for (vk::WriteDescriptorSet &w: writes) w.dstSet = set;
    device.updateDescriptorSets(writes, 0);
    return true;
}

bool DescriptorBuilder::build(vk::Device &device, vk::DescriptorSet &set)
{
    vk::DescriptorSetLayout layout;
    return build(device, set, layout);
}

}    // namespace pivot::graphics

#pragma once

#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{
class DescriptorAllocator
{

public:
    DescriptorAllocator();
    ~DescriptorAllocator();

    void init(vk::Device &newDevice);
    void cleanup();

    void resetPools();
    bool allocate(vk::DescriptorSet &set, vk::DescriptorSetLayout &layout);

private:
    vk::DescriptorPool grabPool();

private:
    vk::Device device;
    vk::DescriptorPool currentPool = VK_NULL_HANDLE;
    std::vector<vk::DescriptorPool> usedPools;
    std::vector<vk::DescriptorPool> freePools;
};

}    // namespace pivot::graphics

#pragma once

#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/types/common.hxx"

namespace pivot::graphics
{
/// @brief This class handle the allocation of DescriptorSet
///
/// This is done either by picking an existing pool or a create a new pool.
class DescriptorAllocator
{

public:
    /// Ctor
    DescriptorAllocator();
    /// Dtor
    ~DescriptorAllocator();

    /// Initialisation
    void init(vk::Device &newDevice);
    /// Destroy all pools
    void cleanup();

    /// Reset all pools
    void resetPools();
    /// @brief Allocate a new descriptor set.
    ///
    /// If the allocation fail once, a new Descriptor pool will be created and the allocation will be retried. If the
    /// allocation fail a second time, it will return false, and should be considered as a critical failure. Otherwise
    /// it will return true.
    bool allocate(vk::DescriptorSet &set, vk::DescriptorSetLayout &layout,
                  const vk::DescriptorSetVariableDescriptorCountAllocateInfo &pExtension = {});

private:
    vk::DescriptorPool grabPool();

private:
    vk::Device device;
    vk::DescriptorPool currentPool = VK_NULL_HANDLE;
    std::vector<vk::DescriptorPool> usedPools;
    std::vector<vk::DescriptorPool> freePools;
};

}    // namespace pivot::graphics

#pragma once

#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{
/// Cache the descriptor sets layout to avoid duplication
class DescriptorLayoutCache
{
private:
    struct DescriptorLayoutInfo {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        bool operator==(const DescriptorLayoutInfo &other) const;
        size_t hash() const;
    };

    struct DescriptorLayoutHash {
        std::size_t operator()(const DescriptorLayoutInfo &k) const { return k.hash(); }
    };

public:
    /// Initialisation
    void init(vk::Device &newDevice);
    /// Destroy Descriptor Set layout
    void cleanup();
    /// If a similar DescriptorSetLayout cannot be found in the cache, a new one will be created.
    vk::DescriptorSetLayout createDescriptorLayout(vk::DescriptorSetLayoutCreateInfo &info);

private:
    std::unordered_map<DescriptorLayoutInfo, vk::DescriptorSetLayout, DescriptorLayoutHash> layoutCache;
    vk::Device device;
};

}    // namespace pivot::graphics

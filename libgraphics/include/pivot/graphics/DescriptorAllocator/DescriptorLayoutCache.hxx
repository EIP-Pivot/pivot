#pragma once

#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{
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
    void init(vk::Device &newDevice);
    void cleanup();

    vk::DescriptorSetLayout createDescriptorLayout(vk::DescriptorSetLayoutCreateInfo &info);

private:
    std::unordered_map<DescriptorLayoutInfo, vk::DescriptorSetLayout, DescriptorLayoutHash> layoutCache;
    vk::Device device;
};

}    // namespace pivot::graphics

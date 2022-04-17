#include "pivot/graphics/DescriptorAllocator/DescriptorLayoutCache.hxx"

namespace pivot::graphics
{

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo &other) const
{
    auto projection = [](const auto &i) {
        return std::make_tuple(i.binding, i.descriptorType, i.descriptorCount, i.stageFlags);
    };
    return std::ranges::equal(bindings, other.bindings, {}, projection, projection);
}

std::size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
{
    std::size_t result = std::hash<std::size_t>()(bindings.size());

    for (const vk::DescriptorSetLayoutBinding &b: bindings) {
        std::size_t binding_hash = b.binding | static_cast<std::uint32_t>(b.descriptorType) << 8 |
                                   b.descriptorCount << 16 | static_cast<std::uint32_t>(b.stageFlags) << 24;
        result ^= std::hash<std::size_t>()(binding_hash);
    }
    return result;
}

DescriptorLayoutCache::DescriptorLayoutCache(vk::Device &device): device_ref(device) {}

DescriptorLayoutCache::~DescriptorLayoutCache() {}

vk::DescriptorSetLayout DescriptorLayoutCache::createDescriptorLayout(vk::DescriptorSetLayoutCreateInfo &info)
{
    DescriptorLayoutInfo layoutinfo;
    layoutinfo.bindings.resize(info.bindingCount);
    std::memcpy(layoutinfo.bindings.data(), info.pBindings, info.bindingCount * sizeof(vk::DescriptorSetLayoutBinding));
    std::ranges::sort(layoutinfo.bindings, {}, [](const auto &i) { return i.binding; });

    auto it = layoutCache.find(layoutinfo);
    if (it != layoutCache.end()) {
        return it->second;
    } else {
        vk::DescriptorSetLayout layout = device_ref.createDescriptorSetLayout(info);
        layoutCache[layoutinfo] = layout;
        return layout;
    }
}

void DescriptorLayoutCache::cleanup()
{
    for (auto pair: layoutCache) vkDestroyDescriptorSetLayout(device_ref, pair.second, nullptr);
}

}    // namespace pivot::graphics

#include "pivot/graphics/DescriptorAllocator/DescriptorLayoutCache.hxx"

namespace pivot::graphics
{

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo &other) const
{
    if (other.bindings.size() != bindings.size()) {
        return false;
    } else {
        for (unsigned i = 0; i < bindings.size(); i++) {
            // Can't use operator== , because some field could be pointing to different ressources, but
            // those ressource doesn't matter in this case
            if (other.bindings.at(i).binding != bindings.at(i).binding) return false;
            if (other.bindings.at(i).descriptorType != bindings.at(i).descriptorType) return false;
            if (other.bindings.at(i).descriptorCount != bindings.at(i).descriptorCount) return false;
            if (other.bindings.at(i).stageFlags != bindings.at(i).stageFlags) return false;
        }
        return true;
    }
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
    layoutinfo.bindings.reserve(info.bindingCount);
    bool isSorted = true;
    int32_t lastBinding = -1;
    for (uint32_t i = 0; i < info.bindingCount; i++) {
        layoutinfo.bindings.push_back(info.pBindings[i]);
        if (static_cast<std::int32_t>(info.pBindings[i].binding) > lastBinding)
            lastBinding = info.pBindings[i].binding;
        else
            isSorted = false;
    }
    if (!isSorted)
        std::ranges::sort(layoutinfo.bindings, [](const auto &a, const auto &b) { return a.binding < b.binding; });

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

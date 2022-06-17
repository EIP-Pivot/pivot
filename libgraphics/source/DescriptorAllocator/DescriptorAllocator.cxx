#include "pivot/graphics/DescriptorAllocator/DescriptorAllocator.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include <Logger.hpp>

namespace pivot::graphics
{
vk::DescriptorPool createPool(vk::Device &device, std::uint32_t count, vk::DescriptorPoolCreateFlags flags)
{
    DEBUG_FUNCTION
    const std::vector<std::pair<vk::DescriptorType, float>> poolSizes = {
        {vk::DescriptorType::eSampler, 0.5f},
        {vk::DescriptorType::eCombinedImageSampler, 4.f},
        {vk::DescriptorType::eSampledImage, 4.f},
        {vk::DescriptorType::eStorageImage, 1.f},
        {vk::DescriptorType::eUniformTexelBuffer, 1.f},
        {vk::DescriptorType::eStorageTexelBuffer, 1.f},
        {vk::DescriptorType::eUniformBuffer, 2.f},
        {vk::DescriptorType::eStorageBuffer, 2.f},
        {vk::DescriptorType::eUniformBufferDynamic, 1.f},
        {vk::DescriptorType::eStorageBufferDynamic, 1.f},
        {vk::DescriptorType::eInputAttachment, 0.5f},
    };

    std::vector<vk::DescriptorPoolSize> sizes;
    std::ranges::transform(poolSizes, std::back_inserter(sizes), [=](const auto &sz) {
        return vk::DescriptorPoolSize{
            .type = sz.first,
            .descriptorCount = uint32_t(sz.second * count),
        };
    });
    vk::DescriptorPoolCreateInfo pool_info = {
        .flags = flags,
        .maxSets = count,
        .poolSizeCount = static_cast<uint32_t>(sizes.size()),
        .pPoolSizes = sizes.data(),
    };
    return device.createDescriptorPool(pool_info);
}

DescriptorAllocator::DescriptorAllocator(vk::Device &device): device_ref(device) {}

DescriptorAllocator::~DescriptorAllocator() {}

void DescriptorAllocator::resetPools()
{
    for (auto &p: usedPools) device_ref.resetDescriptorPool(p);

    freePools = usedPools;
    usedPools.clear();
    currentPool = VK_NULL_HANDLE;
}

bool DescriptorAllocator::allocate(vk::DescriptorSet &set, vk::DescriptorSetLayout &layout)
try {
    DEBUG_FUNCTION
    if (!currentPool) {
        currentPool = grabPool();
        usedPools.push_back(currentPool);
    }

    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = currentPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };

    bool needReallocate = false;
    std::vector<vk::DescriptorSet> allocResult;
    try {
        allocResult = device_ref.allocateDescriptorSets(allocInfo);
    } catch (vk::FragmentedPoolError &) {
        needReallocate = true;
    } catch (vk::OutOfPoolMemoryError &) {
        needReallocate = true;
    }

    /// Pool is full or unavailable, retry with another one
    if (needReallocate) {
        currentPool = grabPool();
        usedPools.push_back(currentPool);
        allocResult = device_ref.allocateDescriptorSets(allocInfo);
    }
    if (allocResult.empty()) return false;
    set = allocResult.front();
    return true;
} catch (vk::Error &e) {
    logger.err("Descriptor Set Allocator") << e.what();
    return false;
}

void DescriptorAllocator::cleanup()
{
    for (auto &p: freePools) device_ref.destroyDescriptorPool(p);
    for (auto &p: usedPools) device_ref.destroyDescriptorPool(p);
}

vk::DescriptorPool DescriptorAllocator::grabPool()
{
    DEBUG_FUNCTION
    if (freePools.size() > 0) {
        vk::DescriptorPool pool = freePools.back();
        freePools.pop_back();
        return pool;
    }
    return createPool(device_ref, 1000, {});
}

}    // namespace pivot::graphics

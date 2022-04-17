#include "pivot/graphics/VulkanAllocator.hxx"

namespace pivot::graphics
{

VulkanAllocator::VulkanAllocator() {}

VulkanAllocator::~VulkanAllocator() {}

void VulkanAllocator::init(const vma::AllocatorCreateInfo &info)
{
    allocator = vma::createAllocator(info);
    properties = info.physicalDevice.getMemoryProperties();
}

AllocatedImage VulkanAllocator::createImage(const vk::ImageCreateInfo &info, const vma::AllocationCreateInfo &allocInfo)
{
    assert(info.extent.depth != 0 && info.extent.height != 0 && info.extent.width != 0);
    AllocatedImage image{
        .format = info.format,
        .size = info.extent,
        .mipLevels = info.mipLevels,
    };
    std::tie(image.image, image.memory) = allocator.createImage(info, allocInfo);
    return image;
}

void VulkanAllocator::dumpStats()
{
    const auto budgets = allocator.getBudget();
    logger.info("Vulkan Allocator") << "-----------------------------------";
    for (const auto &b: budgets) {
        logger.info("Vulkan Allocator") << "VmaBudget.allocationBytes = "
                                        << vk_utils::tools::bytesToString(b.allocationBytes);
        logger.info("Vulkan Allocator") << "VmaBudget.blockBytes = " << vk_utils::tools::bytesToString(b.blockBytes);
        logger.info("Vulkan Allocator") << "VmaBudget.usage = " << vk_utils::tools::bytesToString(b.usage);
        logger.info("Vulkan Allocator") << "VmaBudget.budget = " << vk_utils::tools::bytesToString(b.budget);
    }
    logger.info("Vulkan Allocator") << "-----------------------------------";
}

VulkanAllocator::GPUMemoryStats VulkanAllocator::getStats()
{
    const auto budgets = allocator.getBudget();
    uint64_t usage = 0;
    uint64_t budget = 0;

    for (const auto &b: budgets) {
        usage += b.usage;
        budget += b.budget;
    }
    return {
        .used = usage,
        .free = budget - usage,
    };
}

}    // namespace pivot::graphics

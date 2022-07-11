#include "pivot/graphics/VulkanAllocator.hxx"

namespace pivot::graphics
{

VulkanAllocator::VulkanAllocator() {}

VulkanAllocator::~VulkanAllocator() {}

void VulkanAllocator::init(const vma::AllocatorCreateInfo &info)
{
    allocator = vma::createAllocator(info);
    device = info.device;
    properties = info.physicalDevice.getMemoryProperties();
}

AllocatedImage VulkanAllocator::createImage(const vk::ImageCreateInfo &info, const vma::AllocationCreateInfo &allocInfo)
{
    pivot_assert(info.extent.depth != 0 && info.extent.height != 0 && info.extent.width != 0);
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
    const auto budgets = allocator.getHeapBudgets();
    logger.info("Vulkan Allocator") << "-----------------------------------";
    for (const auto &b: budgets) {
        logger.info("Vulkan Allocator/Statistics")
            << "VmaBudget.allocationBytes = " << vk_utils::tools::bytesToString(b.statistics.allocationBytes);
        logger.info("Vulkan Allocator/Statistics")
            << "VmaBudget.allocationCount = " << vk_utils::tools::bytesToString(b.statistics.allocationCount);
        logger.info("Vulkan Allocator/Statistics")
            << "VmaBudget.blockBytes = " << vk_utils::tools::bytesToString(b.statistics.blockBytes);
        logger.info("Vulkan Allocator/Statistics")
            << "VmaBudget.blockCount = " << vk_utils::tools::bytesToString(b.statistics.blockCount);
        logger.info("Vulkan Allocator") << "VmaBudget.usage = " << vk_utils::tools::bytesToString(b.usage);
        logger.info("Vulkan Allocator") << "VmaBudget.budget = " << vk_utils::tools::bytesToString(b.budget);
    }
    logger.info("Vulkan Allocator") << "-----------------------------------";

    const auto json_string = allocator.buildStatsString(VK_TRUE);
    logger.debug("Vulkan Allocator/JSON") << json_string;
    vk_utils::writeFile(memory_dump_file_name, json_string);
    allocator.freeStatsString(json_string);
}

VulkanAllocator::GPUMemoryStats VulkanAllocator::getStats()
{
    const auto budgets = allocator.getHeapBudgets();
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

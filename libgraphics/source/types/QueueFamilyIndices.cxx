#include "pivot/graphics/types/QueueFamilyIndices.hxx"

#include "pivot/pivot.hxx"

std::unordered_map<vk::QueueFlagBits, std::vector<std::uint32_t>>
getAllAvailableQueues(const vk::PhysicalDevice &device)
{
    std::unordered_map<vk::QueueFlagBits, std::vector<std::uint32_t>> availableQueues;
    auto family_property_list = device.getQueueFamilyProperties();
    for (std::uint32_t i = 0; i < family_property_list.size(); ++i) {
        if (family_property_list.at(i).queueFlags & vk::QueueFlagBits::eGraphics)
            availableQueues[vk::QueueFlagBits::eGraphics].push_back(i);
        if (family_property_list.at(i).queueFlags & vk::QueueFlagBits::eTransfer)
            availableQueues[vk::QueueFlagBits::eTransfer].push_back(i);
        if (family_property_list.at(i).queueFlags & vk::QueueFlagBits::eCompute)
            availableQueues[vk::QueueFlagBits::eCompute].push_back(i);
    }
    return availableQueues;
}

namespace pivot::graphics
{

constexpr bool isSelected(const QueueFamilyIndices &indices, std::uint32_t index) noexcept
{
    return indices.graphicsFamily.value_or(-1) == index || indices.presentFamily.value_or(-1) == index ||
           indices.transferFamily.value_or(-1) == index || indices.computeFamily.value_or(-1) == index;
}

QueueFamilyIndices QueueFamilyIndices::findQueueFamilies(const vk::PhysicalDevice &device,
                                                         const vk::SurfaceKHR &surface)
{
    DEBUG_FUNCTION
    QueueFamilyIndices indices;
    auto availableQueues = getAllAvailableQueues(device);

    // Try to get unique queue
    for (const auto &[type, queues]: availableQueues) {
        for (const auto &i: queues) {
            switch (type) {
                case vk::QueueFlagBits::eGraphics:
                    if (!isSelected(indices, i))
                        indices.graphicsFamily = i;
                    else if (!isSelected(indices, i) && device.getSurfaceSupportKHR(i, surface))
                        indices.presentFamily = i;
                    break;
                case vk::QueueFlagBits::eTransfer:
                    if (!isSelected(indices, i)) indices.transferFamily = i;
                    break;
                case vk::QueueFlagBits::eCompute:
                    if (!isSelected(indices, i)) indices.computeFamily = i;
                    break;
                default: continue;
            }
        }
    }

    // If some queue were not found, fill with whatever available
    if (!indices.isComplete()) {
        for (const auto &[type, queues]: availableQueues) {
            for (const auto &i: queues) {
                switch (type) {
                    case vk::QueueFlagBits::eGraphics: {
                        if (!indices.graphicsFamily.has_value()) indices.graphicsFamily = i;
                        if (!indices.presentFamily.has_value() && device.getSurfaceSupportKHR(i, surface))
                            indices.presentFamily = i;
                    } break;
                    case vk::QueueFlagBits::eTransfer: {
                        if (!indices.transferFamily.has_value()) indices.transferFamily = i;
                    } break;
                    case vk::QueueFlagBits::eCompute: {
                        if (!indices.computeFamily.has_value()) indices.computeFamily = i;
                    } break;
                    default: continue;
                }
            }
        }
    }
    return indices;
}

}    // namespace pivot::graphics

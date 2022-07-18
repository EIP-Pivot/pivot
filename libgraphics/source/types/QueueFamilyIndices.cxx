#include "pivot/graphics/types/QueueFamilyIndices.hxx"

namespace pivot::graphics
{

QueueFamilyIndices QueueFamilyIndices::findQueueFamilies(const vk::PhysicalDevice &device,
                                                         const vk::SurfaceKHR &surface)
{
    QueueFamilyIndices indices;
    auto family_property_list = device.getQueueFamilyProperties();
    for (uint32_t i = 0; i < family_property_list.size() && !indices.isComplete(); ++i) {
        if (family_property_list.at(i).queueFlags & vk::QueueFlagBits::eGraphics) indices.graphicsFamily = i;
        if (family_property_list.at(i).queueFlags & vk::QueueFlagBits::eTransfer) indices.transferFamily = i;
        if (family_property_list.at(i).queueFlags & vk::QueueFlagBits::eCompute) indices.computeFamilty = i;

        if (device.getSurfaceSupportKHR(i, surface)) indices.presentFamily = i;
    }
    return indices;
}

}    // namespace pivot::graphics

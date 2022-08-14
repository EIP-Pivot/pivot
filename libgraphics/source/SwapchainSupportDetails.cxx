#include "pivot/graphics/VulkanSwapchain.hxx"

namespace pivot::graphics
{

vk::SurfaceFormatKHR VulkanSwapchain::SupportDetails::chooseSwapSurfaceFormat() const noexcept
{
    for (const auto &availableFormat: formats)
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return availableFormat;
    return formats.at(0);
}

vk::PresentModeKHR VulkanSwapchain::SupportDetails::chooseSwapPresentMode() const noexcept
{
    for (const auto &availablePresentMode: presentModes)
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) return availablePresentMode;
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanSwapchain::SupportDetails::chooseSwapExtent(const vk::Extent2D &s) const noexcept
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        auto size = s;
        size.width = std::clamp(size.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        size.height = std::clamp(size.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return size;
    }
}

VulkanSwapchain::SupportDetails VulkanSwapchain::SupportDetails::querySwapChainSupport(const vk::PhysicalDevice &device,
                                                                                       const vk::SurfaceKHR &surface)
{
    return VulkanSwapchain::SupportDetails{
        .capabilities = device.getSurfaceCapabilitiesKHR(surface),
        .formats = device.getSurfaceFormatsKHR(surface),
        .presentModes = device.getSurfacePresentModesKHR(surface),
    };
}

}    // namespace pivot::graphics

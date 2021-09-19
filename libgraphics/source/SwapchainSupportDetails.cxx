#include "pivot/graphics/Swapchain.hxx"
#include "pivot/graphics/Window.hxx"

vk::SurfaceFormatKHR Swapchain::SupportDetails::chooseSwapSurfaceFormat() noexcept
{
    for (const auto &availableFormat: formats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    return formats.at(0);
}

vk::PresentModeKHR Swapchain::SupportDetails::chooseSwapPresentMode() noexcept
{
    for (const auto &availablePresentMode: presentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) { return availablePresentMode; }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::SupportDetails::chooseSwapExtent(Window &window) noexcept
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        auto actualExtent = window.getSize();

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

Swapchain::SupportDetails Swapchain::SupportDetails::querySwapChainSupport(const vk::PhysicalDevice &device,
                                                                           const vk::SurfaceKHR &surface)
{
    return Swapchain::SupportDetails{
        .capabilities = device.getSurfaceCapabilitiesKHR(surface),
        .formats = device.getSurfaceFormatsKHR(surface),
        .presentModes = device.getSurfacePresentModesKHR(surface),
    };
}

#pragma once

#include "pivot/graphics/Window.hxx"
#include <algorithm>
#include <vector>
#include <vulkan/vulkan.hpp>

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat()
    {
        for (const auto &availableFormat: formats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
                availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return formats.at(0);
    }

    vk::PresentModeKHR chooseSwapPresentMode()
    {
        for (const auto &availablePresentMode: presentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) { return availablePresentMode; }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapExtent(Window &window)
    {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window.getWindow(), &width, &height);

            vk::Extent2D actualExtent{
                .width = static_cast<uint32_t>(width),
                .height = static_cast<uint32_t>(height),
            };

            actualExtent.width =
                std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height =
                std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    static SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice &device,
                                                         const vk::SurfaceKHR &surface)
    {
        return SwapChainSupportDetails{
            .capabilities = device.getSurfaceCapabilitiesKHR(surface),
            .formats = device.getSurfaceFormatsKHR(surface),
            .presentModes = device.getSurfacePresentModesKHR(surface),
        };
    }
};

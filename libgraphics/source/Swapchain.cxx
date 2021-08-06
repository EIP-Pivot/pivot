#include "pivot/graphics/Swapchain.hxx"

#include <optional>
#include <stddef.h>
#include <stdexcept>

#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/SwapChainSupportDetails.hxx"
#include "pivot/graphics/vk_utils.hxx"

class Window;

Swapchain::Swapchain() {}

Swapchain::~Swapchain() {}

void Swapchain::init(VkExtent2D &windowExtent, VkPhysicalDevice &gpu, VkDevice &device, VkSurfaceKHR &surface)
{
    createSwapchain(windowExtent, gpu, device, surface);
    getImages(device);
    createImageViews(device);
}

void Swapchain::destroy() { chainDeletionQueue.flush(); }

void Swapchain::recreate(VkExtent2D &windowExtent, VkPhysicalDevice &gpu, VkDevice &device, VkSurfaceKHR &surface)
{
    this->destroy();
    this->init(windowExtent, gpu, device, surface);
}

uint32_t Swapchain::nbOfImage() const
{
    if (swapChainImages.size() != swapChainImageViews.size()) [[unlikely]]
        throw std::length_error("swapchain has different ammout of VkImage and VkImageView");
    return swapChainImages.size();
}

void Swapchain::createSwapchain(VkExtent2D &windowExtent, VkPhysicalDevice &gpu, VkDevice &device,
                                VkSurfaceKHR &surface)
{
    auto indices = QueueFamilyIndices::findQueueFamilies(gpu, surface);

    auto swapChainSupport = SwapChainSupportDetails::querySwapChainSupport(gpu, surface);
    auto surfaceFormat = swapChainSupport.chooseSwapSurfaceFormat();
    auto presentMode = swapChainSupport.chooseSwapPresentMode();
    auto extent = swapChainSupport.chooseSwapExtent(windowExtent);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr,
    };
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;        // Optional
        createInfo.pQueueFamilyIndices = nullptr;    // Optional
    }
    VK_TRY(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain));
    chainDeletionQueue.push([&]() { vkDestroySwapchainKHR(device, swapChain, nullptr); });
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void Swapchain::getImages(VkDevice &device)
{
    uint32_t imageCount = 0;

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
}

void Swapchain::createImageViews(VkDevice &device)
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); ++i) {
        auto createInfo = VkImageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .image = swapChainImages.at(i),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapChainImageFormat,
            .components =
                {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };
        VK_TRY(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]));
    }
    chainDeletionQueue.push([&]() {
        for (auto &imageView: swapChainImageViews) { vkDestroyImageView(device, imageView, nullptr); }
    });
}

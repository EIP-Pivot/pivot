#include "pivot/graphics/VulkanSwapchain.hxx"

#include <optional>
#include <stddef.h>
#include <stdexcept>

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/Window.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

VulkanSwapchain::VulkanSwapchain() {}

VulkanSwapchain::~VulkanSwapchain()
{
    if (*this) this->destroy();
}

void VulkanSwapchain::create(const vk::Extent2D &size, vk::PhysicalDevice &gpu, vk::Device &device,
                             vk::SurfaceKHR &surface)
{
    DEBUG_FUNCTION
    this->device = device;
    createSwapchain(size, gpu, surface);
    getImages();
    createImageViews();
}

void VulkanSwapchain::destroy() { chainDeletionQueue.flush(); }

void VulkanSwapchain::recreate(const vk::Extent2D &size, vk::PhysicalDevice &gpu, vk::Device &device,
                               vk::SurfaceKHR &surface)
{
    DEBUG_FUNCTION
    destroy();
    create(size, gpu, device, surface);
}

uint32_t VulkanSwapchain::nbOfImage() const
{
    assert(swapChainImages.size() == swapChainImageViews.size());
    return swapChainImages.size();
}

uint32_t VulkanSwapchain::getNextImageIndex(const uint64_t maxDelay, vk::Semaphore semaphore)
{
    uint32_t imageIndex;
    vk::Result result;

    std::tie(result, imageIndex) = device->acquireNextImageKHR(swapChain, maxDelay, semaphore);
    vk_utils::vk_try(result);
    return imageIndex;
}

void VulkanSwapchain::createSwapchain(const vk::Extent2D &size, vk::PhysicalDevice &gpu, vk::SurfaceKHR &surface)
{
    DEBUG_FUNCTION
    assert(device);

    auto indices = QueueFamilyIndices::findQueueFamilies(gpu, surface);

    auto swapChainSupport = SupportDetails::querySwapChainSupport(gpu, surface);
    auto surfaceFormat = swapChainSupport.chooseSwapSurfaceFormat();
    auto presentMode = swapChainSupport.chooseSwapPresentMode();
    auto extent = swapChainSupport.chooseSwapExtent(size);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr,
    };
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;        // Optional
        createInfo.pQueueFamilyIndices = nullptr;    // Optional
    }

    swapChain = device->createSwapchainKHR(createInfo);
    chainDeletionQueue.push([&]() { device->destroy(swapChain); });
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void VulkanSwapchain::getImages()
{
    DEBUG_FUNCTION;
    assert(swapChain);
    swapChainImages = device->getSwapchainImagesKHR(swapChain);
}

void VulkanSwapchain::createImageViews()
{
    DEBUG_FUNCTION
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); ++i) {
        vk::ImageViewCreateInfo createInfo =
            vk_init::populateVkImageViewCreateInfo(this->getSwapchainImage(i), this->getSwapchainFormat());
        swapChainImageViews.at(i) = device->createImageView(createInfo);
    }
    chainDeletionQueue.push([&]() {
        for (auto &imageView: swapChainImageViews) { device->destroyImageView(imageView); }
    });
}

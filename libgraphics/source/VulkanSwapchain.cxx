#include "pivot/graphics/VulkanSwapchain.hxx"

#include <optional>
#include <stddef.h>

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics
{

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

std::uint32_t VulkanSwapchain::nbOfImage() const
{
    pivot_assert(swapChainImages.size() == swapChainImageViews.size());
    return swapChainImages.size();
}

std::uint32_t VulkanSwapchain::getNextImageIndex(const uint64_t maxDelay, vk::Semaphore semaphore)
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
    pivot_assert(device);

    auto indices = QueueFamilyIndices::findQueueFamilies(gpu, surface);

    auto swapChainSupport = SupportDetails::querySwapChainSupport(gpu, surface);
    auto surfaceFormat = swapChainSupport.chooseSwapSurfaceFormat();
    auto presentMode = swapChainSupport.chooseSwapPresentMode();
    auto extent = swapChainSupport.chooseSwapExtent(size);
    if (!(swapChainSupport.capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst)) {
        logger.err("Vulkan Swapchain") << "Swapchain image does not support "
                                       << vk::to_string(vk::ImageUsageFlagBits::eTransferDst) << "usage";
        throw VulkanSwapchainError("Bad surface capability");
    }

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
    vk_debug::setObjectName(device.value(), swapChain, "Main Swapchain");
    chainDeletionQueue.push([&] { device->destroy(swapChain); });
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void VulkanSwapchain::getImages()
{
    DEBUG_FUNCTION;
    pivot_assert(swapChain);
    swapChainImages = device->getSwapchainImagesKHR(swapChain);

    for (unsigned i = 0; i < swapChainImages.size(); i++) {
        vk_debug::setObjectName(device.value(), swapChainImages[i], "Swapchain Image nb " + std::to_string(i));
    }
}

void VulkanSwapchain::createImageViews()
{
    DEBUG_FUNCTION
    swapChainImageViews.resize(swapChainImages.size());

    for (unsigned i = 0; i < swapChainImages.size(); i++) {
        vk::ImageViewCreateInfo createInfo =
            vk_init::populateVkImageViewCreateInfo(this->getSwapchainImage(i), this->getSwapchainFormat());
        swapChainImageViews.at(i) = device->createImageView(createInfo);
        vk_debug::setObjectName(device.value(), swapChainImageViews.at(i),
                                "Swapchain Image view nb " + std::to_string(i));
    }
    chainDeletionQueue.push([&] {
        for (auto &imageView: swapChainImageViews) { device->destroyImageView(imageView); }
    });
}

}    // namespace pivot::graphics

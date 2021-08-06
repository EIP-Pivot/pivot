#pragma once

#include <stdint.h>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "pivot/graphics/DeletionQueue.hxx"

struct CreationParameters {
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkSampleCountFlagBits msaaSample = VK_SAMPLE_COUNT_1_BIT;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
};

class Swapchain
{
public:
    Swapchain();
    ~Swapchain();

    void init(VkExtent2D &windowExtent, VkPhysicalDevice &gpu, VkDevice &device, VkSurfaceKHR &surface);
    void destroy();
    void recreate(VkExtent2D &windowExtent, VkPhysicalDevice &gpu, VkDevice &device, VkSurfaceKHR &surface);
    uint32_t nbOfImage() const;

    inline const VkSwapchainKHR &getSwapchain() const { return swapChain; }
    inline float getAspectRatio() const
    {
        return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
    }
    inline VkImage &getSwapchainImage(const unsigned index) { return swapChainImages.at(index); }
    inline VkImageView &getSwapchainImageView(const unsigned index) { return swapChainImageViews.at(index); }
    inline const VkFormat &getSwapchainFormat() const { return swapChainImageFormat; }
    inline const VkExtent2D &getSwapchainExtent() const { return swapChainExtent; }
    inline const VkExtent3D getSwapchainExtent3D(uint32_t depth = 1) const
    {
        return {
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .depth = depth,
        };
    }

private:
    void createSwapchain(VkExtent2D &windowExtent, VkPhysicalDevice &gpu, VkDevice &device, VkSurfaceKHR &surface);
    void getImages(VkDevice &device);
    void createImageViews(VkDevice &device);

private:
    DeletionQueue chainDeletionQueue;
    VkExtent2D swapChainExtent = {.width = 0, .height = 0};
    VkFormat swapChainImageFormat;

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
};

#pragma once

#include <optional>
#include <stdint.h>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/DeletionQueue.hxx"

class Window;

/// @class VulkanSwapchain
///
/// @brief Utility class to ease the creation of Vulkan swapchains
class VulkanSwapchain
{
public:
    /// @struct SupportDetails
    ///
    /// @brief Helper class to gather all information required for swapchain creation
    struct SupportDetails {
        /// surface capability
        vk::SurfaceCapabilitiesKHR capabilities;
        /// List of supported format
        std::vector<vk::SurfaceFormatKHR> formats;
        /// List of supported presentation modes
        std::vector<vk::PresentModeKHR> presentModes;

        /// Choose a fitting format
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat() noexcept;
        /// Choose a presentation mode
        vk::PresentModeKHR chooseSwapPresentMode() noexcept;
        /// Check if the size if supported
        vk::Extent2D chooseSwapExtent(const vk::Extent2D &size) noexcept;
        /// Gather swapchain support information
        static SupportDetails querySwapChainSupport(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);
        /// @endcond
    };

public:
    /// Constructor
    VulkanSwapchain();
    /// Destructor
    ~VulkanSwapchain();

    /// Create a new swapchain
    void create(const vk::Extent2D &size, vk::PhysicalDevice &gpu, vk::Device &device, vk::SurfaceKHR &surface);

    /// Destroy the swapchain
    void destroy();

    /// @brief Recreate the swapchain.
    ///
    /// @code
    /// // Shorthand for
    /// swapchain.destroy();
    /// swapchain.init(...);
    /// @endcode
    void recreate(const vk::Extent2D &size, vk::PhysicalDevice &gpu, vk::Device &device, vk::SurfaceKHR &surface);

    /// Return the number of images in the swapchain
    /// @throw std::length_error if the number of images and the number of imageViews are not equal. (Unlikely to
    /// happend)
    uint32_t nbOfImage() const;

    uint32_t getNextImageIndex(const uint64_t maxDelay, vk::Semaphore semaphore);

    /// Return the Vulkan object
    constexpr const vk::SwapchainKHR &getSwapchain() const noexcept { return swapChain; }

    /// Return the swapchain aspect ratio
    constexpr float getAspectRatio() const noexcept
    {
        return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
    }

    /// Return the swapchain image at the requested index
    inline vk::Image &getSwapchainImage(const unsigned index) { return swapChainImages.at(index); }
    /// Return the swapchain image view at the requested index
    inline vk::ImageView &getSwapchainImageView(const unsigned index) { return swapChainImageViews.at(index); }
    /// Return the swapchain format
    constexpr const vk::Format &getSwapchainFormat() const noexcept { return swapChainImageFormat; }
    /// Return the 2D extend of the swapchain
    constexpr const vk::Extent2D &getSwapchainExtent() const noexcept { return swapChainExtent; }
    /// Return the 3D extend of the swapchain
    constexpr const vk::Extent3D getSwapchainExtent3D(uint32_t depth = 1) const noexcept
    {
        return {
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .depth = depth,
        };
    }

    /// See if the swapchain was created or not
    inline operator bool() const noexcept { return swapChain; }

private:
    void createSwapchain(const vk::Extent2D &size, vk::PhysicalDevice &gpu, vk::SurfaceKHR &surface);
    void getImages();
    void createImageViews();

private:
    std::optional<vk::Device> device = std::nullopt;
    DeletionQueue chainDeletionQueue;
    vk::Extent2D swapChainExtent;
    vk::Format swapChainImageFormat;

    vk::SwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::ImageView> swapChainImageViews;
};

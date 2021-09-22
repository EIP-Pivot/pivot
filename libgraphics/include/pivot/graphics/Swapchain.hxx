#pragma once

#include <stdint.h>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/DeletionQueue.hxx"

class Window;

/// @class Swapchain
///
/// @brief Utility class to ease the creation of Vulkan swapchains
class Swapchain
{
public:
    /// @struct SupportDetails
    ///
    /// @brief Helper class to gather all information required for swapchain creation
    struct SupportDetails {
        /// @cond
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;

        vk::SurfaceFormatKHR chooseSwapSurfaceFormat() noexcept;
        vk::PresentModeKHR chooseSwapPresentMode() noexcept;
        vk::Extent2D chooseSwapExtent(Window &window) noexcept;
        static SupportDetails querySwapChainSupport(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);
        /// @endcond
    };

public:
    /// Constructor
    Swapchain();
    /// Destructor
    ~Swapchain();

    /// Create a new swapchain
    void init(Window &win, vk::PhysicalDevice &gpu, vk::Device &device, vk::SurfaceKHR &surface);

    /// Destroy the swapchain
    void destroy();

    /// @brief Recreate the swapchain.
    ///
    /// @code
    /// // Shorthand for
    /// swapchain.destroy();
    /// swapchain.init(...);
    /// @endcode
    void recreate(Window &win, vk::PhysicalDevice &gpu, vk::Device &device, vk::SurfaceKHR &surface);

    /// Return the number of images in the swapchain
    /// @throw std::length_error if the number of images and the number of imageViews are not equal. (Unlikely to
    /// happend)
    uint32_t nbOfImage() const;

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
    void createSwapchain(Window &win, vk::PhysicalDevice &gpu, vk::Device &device, vk::SurfaceKHR &surface);
    void getImages(vk::Device &device);
    void createImageViews(vk::Device &device);

private:
    DeletionQueue chainDeletionQueue;
    vk::Extent2D swapChainExtent;
    vk::Format swapChainImageFormat;

    vk::SwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::ImageView> swapChainImageViews;
};

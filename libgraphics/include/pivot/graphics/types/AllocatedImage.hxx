#pragma once

#include <filesystem>

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

class VulkanBase;

/// @class AllocatedImage
///
/// @brief Utility class to keep track of a Vulkan image,  its allocated memory et ImageView
class AllocatedImage
{
public:
    /// Create the image view for this image
    void createImageView(VulkanBase &base);
    /// @copydoc createImageView
    void createImageView(VulkanBase &base, const vk::ImageViewCreateInfo &info);
    /// Generate mipmaps for the image
    void generateMipmaps(VulkanBase &base, uint32_t mipLevel);
    /// Transition image layout to given format
    void transitionLayout(VulkanBase &i, vk::ImageLayout layout);

public:
    //// @cond
    vk::Image image = VK_NULL_HANDLE;
    vk::ImageView imageView = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
    vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
    vk::Format format;
    vk::Extent3D size = {0, 0};
    uint32_t mipLevels = 1;
    /// @endcond
};

}    // namespace pivot::graphics

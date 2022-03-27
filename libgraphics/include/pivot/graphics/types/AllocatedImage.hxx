#pragma once

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/vk_init.hxx"

namespace pivot::graphics
{

/// @class AllocatedImage
///
/// @brief Utility class to keep track of a Vulkan image,  its allocated memory et ImageView
class AllocatedImage
{
public:
    /// Create the image view for this image
    void createImageView(vk::Device &device)
    {
        createImageView(device, vk_init::populateVkImageViewCreateInfo(image, format, mipLevels));
    }
    /// @copydoc createImageView
    void createImageView(vk::Device &device, const vk::ImageViewCreateInfo &info)
    {
        imageView = device.createImageView(info);
    }

public:
    //// @cond
    vk::Image image = VK_NULL_HANDLE;
    vk::ImageView imageView = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
    vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
    vk::Format format;
    vk::Extent3D size = {0, 0, 1};
    uint32_t mipLevels = 1;
    /// @endcond
};

}    // namespace pivot::graphics

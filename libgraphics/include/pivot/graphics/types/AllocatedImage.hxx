#pragma once

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/pivot.hxx"
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

    /// return the info struct used when creating a descriptor set
    /// @see DescriptorBuilder::bindBuffer
    vk::DescriptorImageInfo getImageInfo(vk::Sampler &sampler) const
    {
        return {
            .sampler = sampler,
            .imageView = imageView,
            .imageLayout = imageLayout,
        };
    }

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

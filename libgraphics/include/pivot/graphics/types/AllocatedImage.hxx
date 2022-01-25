#pragma once

#include <filesystem>

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/abstract/AImmediateCommand.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

namespace pivot::graphics
{

/// @class AllocatedImage
///
/// @brief Utility class to keep track of a Vulkan image,  its allocated memory et ImageView
class AllocatedImage
{
public:
    AllocatedImage();
    AllocatedImage(AllocatedBuffer &buffer);
    ~AllocatedImage();

    void generateMipmaps(VulkanBase &base, vk::Format imageFormat, uint32_t mipLevel);
    void transitionLayout(abstract::AImmediateCommand &i, vk::Format format, vk::ImageLayout layout);
    static void destroy(VulkanBase &base, AllocatedImage &image);

public:
    //// @cond
    vk::Image image = VK_NULL_HANDLE;
    vk::ImageView imageView = VK_NULL_HANDLE;
    vma::Allocation memory = VK_NULL_HANDLE;
    vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
    vk::Extent3D size = {0, 0};
    uint32_t mipLevels = 1;
    /// @endcond
};

}    // namespace pivot::graphics
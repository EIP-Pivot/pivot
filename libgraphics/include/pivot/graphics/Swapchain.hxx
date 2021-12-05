#pragma once

#include "pivot/graphics/common.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// @class Swapchain
/// @brief This is a "virtual" swapchain aka not linked to actual Vulkan swapchain
class Swapchain
{
public:
    /// @struct CreateInfo
    /// @brief Hold all the information used in the creation of the swapchain
    struct CreateInfo {
        /// size of the swapchain images
        vk::Extent3D size;
        /// MSAA sample
        vk::SampleCountFlagBits msaaSamples;
        /// Format of the swapchain images
        vk::Format format = vk::Format::eB8G8R8A8Srgb;
    };

public:
    /// Constructor
    Swapchain();
    /// Destructor
    ~Swapchain();

    /// Create the swapchain
    /// @param info CreateInfo structure holding the info to create the swapchain
    /// @param alloc a valid allocator structure used to allocated images from
    /// @param device a valid vk::Device handle to create ImageView
    void create(const CreateInfo &info, vma::Allocator &alloc, vk::Device &device);

    /// Destroy the swapchain
    void destroy();

    /// Return the number of images in the swapchain
    /// happend)
    uint32_t nbOfImage() const noexcept { return images.size(); }

    /// @cond
    inline const CreateInfo &getInfo() const noexcept { return m_info; }
    inline auto &getImages() const noexcept { return images; }
    inline const vk::Image &getImage(const auto index) const { return images.at(index).image; }
    inline const vk::ImageView &getImageView(const auto index) const { return images.at(index).imageView; }

    inline operator bool() const noexcept { return !images.empty(); }
    /// @endcond

private:
    void createImage();

private:
    CreateInfo m_info;
    std::vector<AllocatedImage> images;
    OptionalRef<vk::Device> device_ref;
    OptionalRef<vma::Allocator> allocator_ref;
};

}    // namespace pivot::graphics
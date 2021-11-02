#pragma once

#include "pivot/graphics/common.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

class Swapchain
{
public:
    struct CreateInfo {
        vk::Extent3D size;
        vk::SampleCountFlagBits msaaSamples;
        vk::Format format = vk::Format::eB8G8R8A8Srgb;
    };

public:
    Swapchain();
    ~Swapchain();

    void create(const CreateInfo &info, vma::Allocator &alloc, vk::Device &device);
    void destroy();

    inline const CreateInfo &getInfo() const noexcept { return m_info; }
    inline auto &getImages() noexcept { return images; }
    inline vk::Image &getImage(const auto index) { return images.at(index).image; }
    inline vk::ImageView &getImageView(const auto index) { return images.at(index).imageView; }

    inline operator bool() const noexcept { return !images.empty(); }

private:
    void createImage();

private:
    CreateInfo m_info;
    std::vector<AllocatedImage> images;
    OptionalRef<vk::Device> device_ref;
    OptionalRef<vma::Allocator> allocator_ref;
};

}    // namespace pivot::graphics
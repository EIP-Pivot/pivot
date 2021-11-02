#include "pivot/graphics/Swapchain.hxx"
#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/common.hxx"
#include "pivot/graphics/vk_init.hxx"

namespace pivot::graphics
{

Swapchain::Swapchain() {}

Swapchain::~Swapchain()
{
    if (*this) destroy();
}

void Swapchain::create(const Swapchain::CreateInfo &info, vma::Allocator &alloc, vk::Device &device)
{
    DEBUG_FUNCTION
    device_ref = device;
    allocator_ref = alloc;
    m_info = info;
    images.resize(PIVOT_MAX_FRAME_FRAME_IN_FLIGHT);
    createImage();
}

void Swapchain::destroy()
{
    DEBUG_FUNCTION
    for (auto &[image, imageView, allocation]: images) {
        device_ref->get().destroyImageView(imageView);
        allocator_ref->get().destroyImage(image, allocation);
    }
    images.resize(0);
}

void Swapchain::createImage()
{
    DEBUG_FUNCTION
    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = m_info.format,
        .extent = m_info.size,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };
    vma::AllocationCreateInfo allocInfo{};
    allocInfo.usage = vma::MemoryUsage::eGpuOnly;
    for (auto &[image, imageView, allocation]: images) {
        std::tie(image, allocation) = allocator_ref->get().createImage(imageInfo, allocInfo);

        auto imageViewCreateInfo = vk_init::populateVkImageViewCreateInfo(image, m_info.format);
        imageView = device_ref->get().createImageView(imageViewCreateInfo);
    }
}

}    // namespace pivot::graphics

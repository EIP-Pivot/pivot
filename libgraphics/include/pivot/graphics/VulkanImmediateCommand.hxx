#pragma once

#include "pivot/exception.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"
#include "pivot/graphics/types/QueueFamilyIndices.hxx"

#include "pivot/utility/flags.hxx"

#include <source_location>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// Used to easly add immediateCommand functionality to an herited class
class VulkanImmediateCommand
{
public:
    /// Immediate Command error
    RUNTIME_ERROR(VulkanImmediateCommand);

private:
    struct QueueData {
        vk::Queue queue;
        vk::CommandPool pool;
    };

public:
    /// Default ctor
    VulkanImmediateCommand();
    /// Default dtor
    ~VulkanImmediateCommand();

    /// Initialize the immediate context, using a device an a queue
    void init(vk::Device &, const vk::PhysicalDevice &gpu, const QueueFamilyIndices &queueFamilyIndex);
    /// Destroy the immediate context and releasing the reference to the device
    void destroy();

    /// Perform an command on the GPU immediately and wait for it to complete
    void immediateCommand(std::function<void(vk::CommandBuffer &)> cmd, vk::QueueFlagBits requiredQueue,
                          const std::source_location &location = std::source_location::current());

    template <typename T>
    /// Copy the source buffer into the destination
    void copyBuffer(AllocatedBuffer<T> &src, AllocatedBuffer<T> &dst, vk::DeviceSize srcOffset = 0,
                    vk::DeviceSize dstOffset = 0)
    {
        if (src.getBytesSize() - srcOffset > dst.getAllocatedSize() - dstOffset)
            throw VulkanImmediateCommandError("The destination buffer is too small");

        immediateCommand(
            [&](vk::CommandBuffer &cmd) {
                vk::BufferCopy copyRegion{
                    .srcOffset = srcOffset,
                    .dstOffset = dstOffset,
                    .size = src.getBytesSize() - srcOffset,
                };
                cmd.copyBuffer(src.buffer, dst.buffer, copyRegion);
            },
            vk::QueueFlagBits::eTransfer);
        dst.size = src.getSize();
    }

    /// Copy buffer to image
    void copyBufferToImage(const AllocatedBuffer<std::byte> &srdBuffer, AllocatedImage &dstImage);
    /// Copy image to image
    void copyImageToImage(const AllocatedImage &srcBuffer, AllocatedImage &dstImage);
    /// Generate mipmaps for the image
    void generateMipmaps(AllocatedImage &image, uint32_t mipLevel);

    /// Transition image layout to given format
    void transitionLayout(AllocatedImage &image, vk::ImageLayout layout);

private:
    void createImmediateContext(const QueueFamilyIndices &queueFamilyIndex);

private:
    OptionalRef<const vk::PhysicalDevice> physical_device_ref;
    OptionalRef<vk::Device> device_ref;

    vk::Fence immediateFence = VK_NULL_HANDLE;
    std::unordered_map<vk::QueueFlagBits, QueueData> queues;
};

}    // namespace pivot::graphics

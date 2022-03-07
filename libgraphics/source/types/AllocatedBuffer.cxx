#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"

#include "pivot/graphics/VulkanBase.hxx"

namespace pivot::graphics
{

AllocatedBuffer::AllocatedBuffer() {}

AllocatedBuffer::~AllocatedBuffer() {}
void AllocatedBuffer::copyToImage(abstract::AImmediateCommand &i, AllocatedImage &dstImage) const
{
    i.immediateCommand([&](vk::CommandBuffer &cmd) {
        vk::BufferImageCopy region{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .imageOffset = {0, 0, 0},
            .imageExtent = dstImage.size,
        };
        cmd.copyBufferToImage(buffer, dstImage.image, vk::ImageLayout::eTransferDstOptimal, region);
    });
}

AllocatedBuffer AllocatedBuffer::cloneBuffer(VulkanBase &i, vk::BufferUsageFlags usage, vma::MemoryUsage memoryUsage)
{
    auto dstBuffer = i.allocator.createBuffer(size, usage, memoryUsage);
    i.immediateCommand([&](vk::CommandBuffer &cmd) {
        vk::BufferCopy copyRegion{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };
        cmd.copyBuffer(buffer, dstBuffer.buffer, copyRegion);
    });
    return dstBuffer;
}

}    // namespace pivot::graphics

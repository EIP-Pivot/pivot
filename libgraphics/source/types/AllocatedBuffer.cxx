#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"

namespace pivot::graphics
{

AllocatedBuffer::AllocatedBuffer() {}

AllocatedBuffer::~AllocatedBuffer() {}

AllocatedBuffer AllocatedBuffer::cloneBuffer(VulkanBase &i, vk::BufferUsageFlags usage, vma::MemoryUsage memoryUsage)
{
    auto dstBuffer = AllocatedBuffer::create(i, size, usage, memoryUsage);
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

AllocatedBuffer::operator bool() const noexcept { return buffer && memory; }

AllocatedBuffer AllocatedBuffer::create(VulkanBase &base, uint32_t allocSize, vk::BufferUsageFlags usage,
                                        vma::MemoryUsage memoryUsage)
{
    if (allocSize == 0) { throw std::runtime_error("Can't allocated an empty buffer !"); }
    vk::BufferCreateInfo bufferInfo{
        .size = allocSize,
        .usage = usage,
    };
    vma::AllocationCreateInfo vmaallocInfo;
    vmaallocInfo.usage = memoryUsage;
    AllocatedBuffer buffer;
    buffer.size = allocSize;
    std::tie(buffer.buffer, buffer.memory) = base.allocator.createBuffer(bufferInfo, vmaallocInfo);
    return buffer;
}

void AllocatedBuffer::destroy(VulkanBase &base, AllocatedBuffer &buffer)
{
    if (buffer) base.allocator.destroyBuffer(buffer.buffer, buffer.memory);
}

}    // namespace pivot::graphics
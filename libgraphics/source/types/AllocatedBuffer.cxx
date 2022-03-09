#include "pivot/graphics/types/AllocatedBuffer.hxx"

#include "pivot/graphics/VulkanBase.hxx"

namespace pivot::graphics
{

AllocatedBuffer AllocatedBuffer::cloneBuffer(VulkanBase &i, vk::BufferUsageFlags usage, vma::MemoryUsage memoryUsage,
                                             uint32_t size)
{
    if (size == 0) size = getSize();
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

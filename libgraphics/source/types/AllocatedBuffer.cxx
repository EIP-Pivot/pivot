#include "pivot/graphics/types/AllocatedBuffer.hxx"

namespace pivot::graphics
{

AllocatedBuffer::AllocatedBuffer() {}

AllocatedBuffer::~AllocatedBuffer() {}

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
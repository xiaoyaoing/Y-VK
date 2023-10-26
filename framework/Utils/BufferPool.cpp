#include "BufferPool.h"

BufferAllocation BufferPool::AllocateBufferBlock(VkDeviceSize allocateSize)
{
    assert(allocateSize+offset<size);
    BufferAllocation allocation = {.buffer = buffer.get(), .offset = offset, .size = allocateSize};
    offset += allocateSize;
    return allocation;
}

BufferPool::BufferPool(Device& device, VkDeviceSize size, VkBufferUsageFlags usage,
                       VmaMemoryUsage memoryUsage): size(size)
{
    buffer = std::make_unique<Buffer>(device, size, usage, memoryUsage);
}

void BufferPool::reset()
{
    offset = 0;
}

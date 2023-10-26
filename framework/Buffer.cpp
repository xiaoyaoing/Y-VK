#include "Buffer.h"
#include <Device.h>


void Buffer::cleanup()
{
    vmaDestroyBuffer(_allocator, _buffer, _bufferAllocation);
}

void Buffer::uploadData(const void* srcData, uint64_t size, uint64_t offset)
{
    assert(srcData != nullptr); // snowapril : source data must not be invalid
    void* dstData{nullptr};
    vmaMapMemory(_allocator, _bufferAllocation, &dstData);
    memcpy(static_cast<char*>(dstData) + offset, srcData, static_cast<size_t>(size));
    vmaUnmapMemory(_allocator, _bufferAllocation);
}

Buffer::Buffer(Device& device, uint64_t bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage)
    : device(device)
{
    _allocator = device.getMemoryAllocator();
    _allocatedSize = bufferSize;
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = bufferSize;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = bufferUsage;

    VmaAllocationCreateInfo bufferAllocInfo = {};
    bufferAllocInfo.usage = memoryUsage;

    VK_CHECK_RESULT(vmaCreateBuffer(_allocator, &bufferInfo, &bufferAllocInfo, &_buffer, &_bufferAllocation, nullptr))
}

Buffer::~Buffer()
{
    vmaDestroyBuffer(_allocator, _buffer, _bufferAllocation);
}

#include "Buffer.h"

Buffer::Buffer(VmaAllocator allocator, uint64_t bufferSize, VkBufferUsageFlags bufferUsage,
               VmaMemoryUsage memoryUsage) {
    if(!initialize(allocator,bufferSize,bufferUsage,memoryUsage)){
        std::runtime_error("error");
    }
}

bool Buffer::initialize(VmaAllocator allocator, uint64_t bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage)
{
    _allocator = allocator;
    _allocatedSize = bufferSize;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = bufferSize;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = bufferUsage;

    VmaAllocationCreateInfo bufferAllocInfo = {};
    bufferAllocInfo.usage = memoryUsage;

    if (vmaCreateBuffer(_allocator, &bufferInfo, &bufferAllocInfo, &_buffer, &_bufferAllocation, nullptr) != VK_SUCCESS)
    {
        return false;
    }
    return true;
}

void Buffer::cleanup() {
    vmaDestroyBuffer(_allocator,_buffer,_bufferAllocation);
}

void Buffer::uploadData(const void *srcData, uint64_t size) {
    assert(srcData != nullptr); // snowapril : source data must not be invalid
    void* dstData{ nullptr };
    vmaMapMemory(_allocator, _bufferAllocation, &dstData);
    memcpy(dstData, srcData, static_cast<size_t>(size));
    vmaUnmapMemory(_allocator, _bufferAllocation);
}

VkBuffer Buffer::getHandle() const {
    return _buffer;
}

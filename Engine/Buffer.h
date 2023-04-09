#pragma once
#include "Vulkan.h"
class Buffer {

    VkBuffer _buffer{nullptr};
    uint32_t _allocatedSize;
    VmaAllocator _allocator  {nullptr};
    VmaAllocation	_bufferAllocation	{	nullptr		 };

    bool
    initialize(VmaAllocator allocator, uint64_t bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);

public:
    Buffer() {}

    explicit Buffer(VmaAllocator allocator, uint64_t bufferSize, VkBufferUsageFlags bufferUsage,
                    VmaMemoryUsage memoryUsage);

    void uploadData		(const void* srcData, uint64_t size);
    void cleanup();
    inline  VkBuffer getHandle() {
        return _buffer;
    }
    inline  VkDeviceSize  getSize(){
        return _allocatedSize;
    }
};
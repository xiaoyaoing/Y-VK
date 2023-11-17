#pragma once

#include "Vulkan.h"

class Device;

class Buffer
{
    VkBuffer _buffer{nullptr};
    uint32_t _allocatedSize;
    VmaAllocator _allocator{nullptr};
    VmaAllocation _bufferAllocation{nullptr};
    Device& device;

public:
    ~Buffer();

    explicit Buffer(Device& device, uint64_t bufferSize, VkBufferUsageFlags bufferUsage,
                    VmaMemoryUsage memoryUsage);

    Buffer(Buffer&& buffer) noexcept;

    void uploadData(const void* srcData, uint64_t size, uint64_t offset = 0);

    void cleanup();

    inline VkBuffer getHandle() const
    {
        return _buffer;
    }

    inline VkDeviceSize getSize()
    {
        return _allocatedSize;
    }
};

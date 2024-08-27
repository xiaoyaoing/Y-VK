#pragma once

#include "Buffer.h"
#include "Device.h"

struct SubAlloc{
    VkDeviceSize offset;
    VkDeviceSize length;

    SubAlloc() : offset(0), length(0) {}
    SubAlloc(VkDeviceSize offset, VkDeviceSize length) : offset(offset), length(length) {}
};

class TransientBuffer {
public:
    TransientBuffer(Device &device, 
                    VkBufferUsageFlags usageFlags,
                    VkDeviceSize initialSize,
                    VkDeviceSize sizeInByte);
    ~TransientBuffer();
    
    SubAlloc allocate(VkDeviceSize size, CommandBuffer& commandBuffer);
    // deallocate the sub alloc
    void deallocate(SubAlloc const & subAlloc);

    inline VkBuffer getBuffer() const { return _buffer->getHandle(); }
    inline VkDeviceAddress getDeviceAddress() const { return _buffer->getDeviceAddress(); }

private:
    Device& _device;
    std::unique_ptr<Buffer> _buffer;
    std::list<SubAlloc> _freeList;
};
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
                    VmaMemoryUsage memoryUsage,const void * data = nullptr);
    Buffer(Buffer&& buffer);
    Buffer(Buffer & rhs ) = delete;
    Buffer operator = (Buffer & rhs) = delete;
    void copyFrom(Buffer & srcBuffer);
    void uploadData(const void* srcData, uint64_t size, uint64_t offset = 0);
    void cleanup();

    template<typename  T>
    std::vector<T> getData()
    {
        std::vector<T> result;
        result.resize(_allocatedSize / sizeof(T));
        
        void *dstData{nullptr};
        vmaMapMemory(_allocator, _bufferAllocation, &dstData);
        memcpy(result.data(),dstData, _allocatedSize);
        vmaUnmapMemory(_allocator, _bufferAllocation);
        return result;
    }

    VkDeviceSize getDeviceAddress() const;
    inline VkBuffer getHandle() const {return _buffer; }
    inline VkDeviceSize getSize() const { return _allocatedSize;}
};

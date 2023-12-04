#include "Buffer.h"
#include "Core/Device/Device.h"


void Buffer::cleanup() {
    vmaDestroyBuffer(_allocator, _buffer, _bufferAllocation);
}

VkDeviceSize Buffer::getDeviceAddress() const
{
    VkBufferDeviceAddressInfo info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
    info.buffer = _buffer;
    return  vkGetBufferDeviceAddress(device.getHandle(), &info);
}

void Buffer::uploadData(const void *srcData, uint64_t size, uint64_t offset) {
    assert(srcData != nullptr); // snowapril : source data must not be invalid
    void *dstData{nullptr};
    vmaMapMemory(_allocator, _bufferAllocation, &dstData);
    memcpy(static_cast<char *>(dstData) + offset, srcData, static_cast<size_t>(size));
    vmaUnmapMemory(_allocator, _bufferAllocation);
}

Buffer::Buffer(Device &device, uint64_t bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage,const void * data)
        : device(device) {
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
    
    if(data)
    {
        uploadData(data, bufferSize);
    }
}

Buffer::Buffer(Buffer &&buffer) : _buffer(buffer._buffer), _allocatedSize(buffer._allocatedSize),
                                  _allocator(buffer._allocator), _bufferAllocation(buffer._bufferAllocation),
                                  device(buffer.device) {
    buffer._buffer = VK_NULL_HANDLE;
}

Buffer::~Buffer() {
    vmaDestroyBuffer(_allocator, _buffer, _bufferAllocation);
}

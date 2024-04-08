#include "Buffer.h"
#include "Core/Device/Device.h"

void Buffer::cleanup() {
    vmaDestroyBuffer(_allocator, _buffer, _bufferAllocation);
}

VkDeviceSize Buffer::getDeviceAddress() const {
    VkBufferDeviceAddressInfo info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
    info.buffer                    = _buffer;
    return vkGetBufferDeviceAddress(device.getHandle(), &info);
}

void Buffer::uploadData(const void* srcData, uint64_t size, uint64_t offset) {
    if (size == -1)
        size = _allocatedSize;
    assert(srcData != nullptr);// snowapril : source data must not be invalid
    void* dstData{nullptr};
    vmaMapMemory(_allocator, _bufferAllocation, &dstData);
    memcpy(static_cast<char*>(dstData) + offset, srcData, static_cast<size_t>(size));
    vmaUnmapMemory(_allocator, _bufferAllocation);
}

Buffer::Buffer(Device& device, uint64_t bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, const void* data)
    : device(device) {
    _allocator                    = device.getMemoryAllocator();
    _allocatedSize                = bufferSize;
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext              = nullptr;
    bufferInfo.size               = bufferSize;
    bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage              = bufferUsage;

    VmaAllocationCreateInfo bufferAllocInfo = {};
    bufferAllocInfo.usage                   = memoryUsage;

    VK_CHECK_RESULT(vmaCreateBuffer(_allocator, &bufferInfo, &bufferAllocInfo, &_buffer, &_bufferAllocation, nullptr))

    if (data) {
        uploadData(data, bufferSize);
    }

    //LOGI("Buffer created: %d bytes, usage %d, memory usage %d", bufferSize, bufferUsage, memoryUsage);
}

Buffer::Buffer(Buffer&& buffer) : _buffer(buffer._buffer), _allocatedSize(buffer._allocatedSize),
                                  _allocator(buffer._allocator), _bufferAllocation(buffer._bufferAllocation),
                                  device(buffer.device) {
    buffer._buffer = VK_NULL_HANDLE;
}


Buffer::~Buffer() {
    if (_buffer != VK_NULL_HANDLE) {
        LOGI("Buffer destroyed")
        vmaDestroyBuffer(_allocator, _buffer, _bufferAllocation);
    }
}
std::unique_ptr<Buffer> Buffer::FromBuffer(Device & device,CommandBuffer& commandBuffer, const Buffer& srcBuffer, VkBufferUsageFlags usageFlags, VkDeviceSize offset) {
        std::unique_ptr<Buffer> dstBuffer = std::make_unique<Buffer>(device, srcBuffer.getSize(), usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        VkBufferCopy            copy{.srcOffset = 0, .size = srcBuffer.getSize()};
        vkCmdCopyBuffer(commandBuffer.getHandle(), srcBuffer.getHandle(), dstBuffer->getHandle(), 1, &copy);
        return dstBuffer;
}
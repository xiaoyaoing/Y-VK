#pragma once

#include "Vulkan.h"

#include "Core/Buffer.h"

struct BufferAllocation {
    Buffer *buffer{nullptr};

    VkDeviceSize offset{0};

    VkDeviceSize size{0};
};

struct BufferBlock {
    VkDeviceSize getFreeSize() const;

    Buffer buffer;

    VkDeviceSize blockSize{0};

    VkDeviceSize offset{0};

    VkDeviceSize alignment{0};

    BufferAllocation allocate(VkDeviceSize size);

    void reset();

    BufferBlock(Device &device, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
};


class BufferPool {
public:
    BufferAllocation AllocateBufferBlock(VkDeviceSize allocateSize);

    BufferPool(Device &device, VkDeviceSize blockSize, VkBufferUsageFlags usage,
               VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU);

    BufferPool(BufferPool &other) = delete;

    void reset();

private:
    std::vector<std::unique_ptr<BufferBlock>> bufferBlocks;


    //VkDeviceSize getFreeSize() const;

    // std::unique_ptr<Buffer> buffer{nullptr};

    Device &device;
    VmaMemoryUsage memoryUsage;
    VkBufferUsageFlags usage;

    VkDeviceSize blockSize{0};

    // VkDeviceSize alignmentSize{0};

    uint32_t activeBlockIdx{0};
};

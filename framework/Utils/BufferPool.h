#pragma once

#include <Vulkan.h>

#include "Buffer.h"

struct BufferAllocation
{
    Buffer* buffer;

    VkDeviceSize offset;

    VkDeviceSize size;
};

struct BufferBlock
{
    VkDeviceSize getFreeSize() const;

    Buffer buffer;

    VkDeviceSize blockSize{0};

    VkDeviceSize offset{0};

    VkDeviceSize alignmentSize{0};
};

class BufferPool
{
public:
    BufferAllocation AllocateBufferBlock(VkDeviceSize allocateSize);

    BufferPool(Device& device, VkDeviceSize size, VkBufferUsageFlags usage,
               VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU);

    BufferPool(BufferPool& other) = delete;

    void reset();

private:
    std::vector<std::unique_ptr<BufferBlock>> bufferBlocks;


    //VkDeviceSize getFreeSize() const;

    std::unique_ptr<Buffer> buffer{nullptr};

    VkDeviceSize size{0};

    VkDeviceSize offset{0};

    VkDeviceSize alignmentSize{0};
};

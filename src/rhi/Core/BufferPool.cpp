#include "BufferPool.h"

#include "Core/Device/Device.h"

BufferAllocation BufferPool::AllocateBufferBlock(VkDeviceSize allocateSize) {
    auto it = std::upper_bound(bufferBlocks.begin(), bufferBlocks.end(), allocateSize,
                               [](const VkDeviceSize &a, const std::unique_ptr<BufferBlock> &b) -> bool {
                                   return a <= b->getFreeSize();
                               });

    if (it != bufferBlocks.end()) {
        return it->get()->allocate(allocateSize);
    }

    bufferBlocks.emplace_back(
            std::make_unique<BufferBlock>(device, std::max(blockSize, allocateSize), usage, memoryUsage));
    return bufferBlocks.back()->allocate(allocateSize);
}

BufferPool::BufferPool(Device &device, VkDeviceSize blockSize, VkBufferUsageFlags usage,
                       VmaMemoryUsage memoryUsage) : device(device), memoryUsage(memoryUsage), usage(usage) {
    // buffer = std::make_unique<Buffer>(device, size, usage, memoryUsage);
}

void BufferPool::reset() {
    for (auto &block: bufferBlocks)
        block->reset();
    activeBlockIdx = 0;
}

BufferAllocation BufferBlock::allocate(VkDeviceSize size) {
    const auto alignOffset = (offset + alignment - 1) & ~(alignment - 1);

    if (alignOffset + size > blockSize) {
        return BufferAllocation{nullptr, 0, 0};
    }
    offset = alignOffset + size;
    return {&buffer, alignOffset, size};
}

VkDeviceSize BufferBlock::getFreeSize() const {
    return blockSize - offset;
}


void BufferBlock::reset() {
    offset = 0;
}

BufferBlock::BufferBlock(Device &device, VkDeviceSize size, VkBufferUsageFlags usage,
                         VmaMemoryUsage memory_usage) : buffer(device, size, usage, memory_usage), blockSize(size) {
    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
        alignment = device.getProperties().limits.minUniformBufferOffsetAlignment;
    } else if (usage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
        alignment = device.getProperties().limits.minStorageBufferOffsetAlignment;
    } else if (usage == VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) {
        alignment = device.getProperties().limits.minTexelBufferOffsetAlignment;
    } else if (usage == VK_BUFFER_USAGE_INDEX_BUFFER_BIT || usage == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT || usage ==
                                                                                                          VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) {
        // Used to calculate the offset, required when allocating memory (its value should be power of 2)
        alignment = 16;
    } else {
        throw std::runtime_error("Usage not recognised");
    }
}

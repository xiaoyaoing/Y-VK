#pragma once

#include "Core/Vulkan.h"
#include "Core/CommandBuffer.h"

class Device;

class Queue;

class CommandPool {
public:
    VkCommandPool inline getHandle() {
        return _pool;
    }

//    CommandPool(ptr<Device> device, ptr<Queue> queue, VkCommandPoolCreateFlags flgas);

    CommandPool(Device &device, uint32_t queueFamilyIndex,
                CommandBuffer::ResetMode resetMode = CommandBuffer::ResetMode::ResetPool);

    CommandBuffer allocateCommandBuffer(VkCommandBufferLevel level, bool begin = false) const;

protected:
    VkCommandPool _pool;
    Device &_device;

};

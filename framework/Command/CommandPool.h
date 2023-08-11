#pragma once

#include "Vulkan.h"
#include <CommandBuffer.h>

class Device;

class Queue;

class CommandPool {
public:
    VkCommandPool inline getHandle() {
        return _pool;
    }

    CommandPool(ptr<Device> device, ptr<Queue> queue, VkCommandPoolCreateFlags flgas);

    CommandPool(Device &device, uint32_t queueFamilyIndex,
                CommandBuffer::ResetMode resetMode = CommandBuffer::ResetMode::ResetPool);

    VkCommandBuffer allocateCommandBuffer();

protected:
    VkCommandPool _pool;
    ptr<Device> _device;

};

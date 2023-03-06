#pragma once
#include <Vulkan.h>
class Device;
class Queue;
class CommandPool {
public:
    VkCommandPool  inline getHandle() {
        return _pool;
    }

    CommandPool(ptr<Device> device,ptr<Queue> queue,VkCommandPoolCreateFlags flgas);
    VkCommandBuffer allocateCommandBuffer();
protected:
    VkCommandPool  _pool;
    ptr<Device> _device;

};

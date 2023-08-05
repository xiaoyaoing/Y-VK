#pragma once
#include "Vulkan.h"
#include <unordered_map>
class Queue;
class Device
{
public:
    inline VkDevice getHandle()
    {
        return _device;
    }
    inline VkPhysicalDevice getPhysicalDevice()
    {
        return _physicalDevice;
    }
    Device(VkPhysicalDevice physicalDevice,
           VkSurfaceKHR surface,
           std::unordered_map<const char *, bool> requested_extensions = {});
    const Queue &getQueueByFlag(VkQueueFlagBits requiredFlag, uint32_t queueIndex);
    const Queue &getPresentQueue(uint32_t queueIndex);
    ~Device() {}
    VmaAllocator getMemoryAllocator() const
    {
        return allocator;
    }
    void setMemoryAllocator(VmaAllocator _allocator)
    {
        allocator = _allocator;
    }

protected:
    VmaAllocator allocator;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    std::vector<std::vector<Queue>> queues;
};
#pragma once
#include "Vulkan.h"
#include <unordered_map>
class Queue;
class Device{
public:

    inline VkDevice getHandle(){
        return _device;
    }
    inline VkPhysicalDevice getPhysicalDevice(){
        return _physicalDevice;
    }
    Device(VkPhysicalDevice  physicalDevice,
            VkSurfaceKHR                           surface,
        std::unordered_map<const char *, bool> requested_extensions = {});
    ptr<Queue> getQueueByFlag(VkQueueFlagBits requiredFlag,uint32_t queueIndex);
    ptr<Queue> getPresentQueue(uint32_t queueIndex);
protected:

    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    std::vector<std::vector<ptr<Queue>>> queues;

};
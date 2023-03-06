#pragma once
#include "Vulkan.h"
class Device{
public:
    inline VkDevice getHandle(){
        return _device;
    }
    Device(VkPhysicalDevice device);
    Device(VkDevice device){_device = device;}
protected:
    VkDevice _device;
};
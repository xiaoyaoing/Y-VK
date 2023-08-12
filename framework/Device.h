#pragma once

#include "Vulkan.h"
#include <unordered_map>

#include <Queue.h>

class Device {
public:
    inline VkDevice getHandle() {
        return _device;
    }

    inline VkPhysicalDevice getPhysicalDevice() {
        return _physicalDevice;
    }

    Device(Device &other) = delete;

    Device operator=(Device other) = delete;

    Device(VkPhysicalDevice physicalDevice,
           VkSurfaceKHR surface,
           std::unordered_map<const char *, bool> requested_extensions = {});

    const Queue &getQueueByFlag(VkQueueFlagBits requiredFlag, uint32_t queueIndex);

    const Queue &getPresentQueue(uint32_t queueIndex);

    ~Device() = default;

    VmaAllocator getMemoryAllocator() const {
        return allocator;
    }

    void setMemoryAllocator(VmaAllocator _allocator) {
        allocator = _allocator;
    }

protected:
    VmaAllocator allocator;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    std::vector<std::vector<std::unique_ptr<Queue>>> queues{};
};
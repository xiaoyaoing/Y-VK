#pragma once

#include "Core/Vulkan.h"
#include "Core/CommandPool.h"
#include <unordered_map>

#include "Core/Queue.h"
// #include <Common/ResourceCache.h>

class ResourceCache;

class Device {
public:
    inline VkDevice getHandle() {
        return _device;
    }

    inline VkPhysicalDevice getPhysicalDevice() {
        return _physicalDevice;
    }

    bool isImageFormatSupported(VkFormat format);

    Device(Device &other) = delete;

    Device operator=(Device other) = delete;

    Device(VkPhysicalDevice physicalDevice,
           VkSurfaceKHR surface,
           std::unordered_map<const char *, bool> requested_extensions = {});

    Queue &getQueueByFlag(VkQueueFlagBits requiredFlag, uint32_t queueIndex);

    const Queue &getPresentQueue(uint32_t queueIndex);

    ~Device() = default;

    VmaAllocator getMemoryAllocator() const {
        return allocator;
    }

    void setMemoryAllocator(VmaAllocator _allocator) {
        allocator = _allocator;
    }

    CommandPool &getCommandPool() {
        return *commandPool;
    }


    CommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false) {
        return commandPool->allocateCommandBuffer(level, begin);
    }

    ResourceCache &getResourceCache();

    VkPhysicalDeviceProperties getProperties() const;

    void waitIdle();

protected:
    VmaAllocator allocator;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    std::vector<std::vector<std::unique_ptr<Queue>>> queues{};

    std::vector<VkExtensionProperties> deviceExtensions{};

    std::unique_ptr<CommandPool> commandPool;

    std::vector<const char *> enabled_extensions{};

    VkPhysicalDeviceProperties properties{};

    ResourceCache *cache;

    bool isExtensionSupported(const std::string &extensionName);
};

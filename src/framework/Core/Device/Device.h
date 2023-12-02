#pragma once

#include "Core/Vulkan.h"
#include "Core/CommandPool.h"
#include <unordered_map>

#include "Core/Queue.h"
// #include <Common/ResourceCache.h>

class ResourceCache;





class Device {
public:
    
    bool isImageFormatSupported(VkFormat format);
    Device(Device &other) = delete;
    Device operator=(Device other) = delete;
    Device(VkPhysicalDevice physicalDevice,
           VkSurfaceKHR surface,
           VkInstance instance,
           std::unordered_map<const char *, bool> requested_extensions = {});
    Queue &getQueueByFlag(VkQueueFlagBits requiredFlag, uint32_t queueIndex);
    const Queue &getPresentQueue(uint32_t queueIndex);
    ~Device() = default;
    
    inline VmaAllocator getMemoryAllocator() const {  return allocator;}
    inline CommandPool & getCommandPool() { return *commandPool;}
    inline CommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false) { return commandPool->allocateCommandBuffer(level, begin);}
    inline ResourceCache &getResourceCache(){    return *cache; }
    inline VkPhysicalDeviceProperties getProperties() const {return properties;}
    inline VkDevice getHandle() const { return _device; }
    inline VkPhysicalDevice getPhysicalDevice() { return _physicalDevice; }
    inline void waitIdle(){     vkDeviceWaitIdle(_device); }

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

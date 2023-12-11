#pragma once

#include "Core/Vulkan.h"
#include "Core/CommandPool.h"
#include <unordered_map>

#include "Core/Queue.h"
// #include <Common/ResourceCache.h>

class ResourceCache;





class Device {
public:

    Device(VkPhysicalDevice physicalDevice,
           VkSurfaceKHR surface,
           VkInstance instance,
           std::unordered_map<const char *, bool> requested_extensions = {});
    Device(Device &other) = delete;
    Device operator=(Device other) = delete;
    ~Device() = default;
    
    bool isImageFormatSupported(VkFormat format);
    
    Queue &getQueueByFlag(VkQueueFlagBits requiredFlag, uint32_t queueIndex);
    const Queue &getPresentQueue(uint32_t queueIndex);
    CommandPool & getCommandPool(VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT) { return commandPools.at(queueFlags);}

    
    inline VmaAllocator getMemoryAllocator() const {  return allocator;}
    inline CommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false) { return getCommandPool().allocateCommandBuffer(level, begin);}
    inline ResourceCache &getResourceCache(){    return *cache; }
    inline VkPhysicalDeviceProperties getProperties() const {return properties;}
    inline VkDevice getHandle() const { return _device; }
    inline VkPhysicalDevice getPhysicalDevice() { return _physicalDevice; }
    inline void waitIdle(){     vkDeviceWaitIdle(_device); }
    inline VkPhysicalDeviceRayTracingPipelinePropertiesKHR & getVkPhysicalDeviceRayTracingPipelineProperties(){return rayTracingPipelineProperties;}

protected:
    VkDevice _device;
    VkPhysicalDevice _physicalDevice;
    VmaAllocator allocator;
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};

    
    std::vector<std::vector<std::unique_ptr<Queue>>> queues{};
    std::vector<VkExtensionProperties> deviceExtensions{};
    std::vector<const char *> enabled_extensions{};

    std::unordered_map<VkQueueFlags, CommandPool> commandPools;
    std::unique_ptr<CommandPool> commandPool;
    ResourceCache *cache;

    bool isExtensionSupported(const std::string &extensionName);
};

#include <GLFW/glfw3.h>
#include "Device.h"
#include <Utils/DebugUtils.h>
#include "Queue.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

Device::Device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
               std::unordered_map<const char *, bool> requiredExtensions)
{
    _physicalDevice = physicalDevice;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    // todo support device features;
    const VkPhysicalDeviceFeatures features{};

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(queueFamilyCount);
    // todo support queuePriority
    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++)
    {
        const auto &queueProp = queueFamilyProperties[queueFamilyIndex];
        std::vector<float> queuePriority(queueProp.queueCount, 0.5f);
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = queueProp.queueCount;
        queueCreateInfo.pQueuePriorities = queuePriority.data();
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    // pointers to the queue creation info and device features structs
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pEnabledFeatures = &features;
    // enable the device extensions todo checkExtension supported
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VK_CHECK_RESULT(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device));

    queues.resize(queueFamilyCount);
    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, queueFamilyIndex, surface, &presentSupport);
        for (uint32_t i = 0; i < queueCreateInfos[queueFamilyIndex].queueCount; i++)
        {
            const VkQueueFamilyProperties &queueFamilyProp = queueFamilyProperties[queueFamilyIndex];
            queues[queueFamilyIndex].emplace_back(
                std::move(std::make_unique<Queue>(this, queueFamilyIndex, i, presentSupport, queueFamilyProp)));
        }
    }
}

const Queue &Device::getQueueByFlag(VkQueueFlagBits requiredFlag, uint32_t queueIndex)
{
    for (const auto &queueFamily : queues)
    {
        const auto &prop = queueFamily[0]->getProp();
        auto queueFlag = prop.queueFlags;
        auto queueCount = prop.queueCount;
        if ((queueFlag & requiredFlag) && queueIndex < queueCount)
            return *queueFamily[queueIndex];
    }
    RUN_TIME_ERROR("Failed to find required queue");
    Queue *queue;
    return *queue;
    // return *(queues[0][0]);
}

const Queue &Device::getPresentQueue(uint32_t queueIndex)
{
    for (const auto &queueFamily : queues)
    {
        const auto &prop = queueFamily[0]->getProp();
        auto canPresent = queueFamily[0]->supportPresent();
        auto queueCount = prop.queueCount;
        if (canPresent && queueIndex < queueCount)
            return *queueFamily[queueIndex];
    }
    RUN_TIME_ERROR("Failed to find present queue");

    Queue *queue;
    return *queue;
    // return *(queues[0][0]);
}

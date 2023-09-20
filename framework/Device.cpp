#include <GLFW/glfw3.h>
#include "Device.h"
#include <Utils/DebugUtils.h>
#include "Queue.h"
#include "Command/CommandPool.h"

#define VMA_IMPLEMENTATION


#include <vk_mem_alloc.h>

const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

Device::Device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
               std::unordered_map<const char*, bool> requiredExtensions)
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
    std::vector<std::vector<float>> queuePrioritys(queueFamilyCount);

    // todo support queuePriority
    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++)
    {
        const auto& queueProp = queueFamilyProperties[queueFamilyIndex];
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = queueProp.queueCount;
        queuePrioritys[queueFamilyIndex].resize(queueProp.queueCount, 0.5);
        queueCreateInfo.pQueuePriorities = queuePrioritys[queueFamilyIndex].data();
        queueCreateInfos.push_back(queueCreateInfo);
    }

    uint32_t device_extension_count;
    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &device_extension_count, nullptr));
    deviceExtensions = std::vector<VkExtensionProperties>(device_extension_count);
    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &device_extension_count,
        deviceExtensions.data()));

    // Display supported extensions
    if (deviceExtensions.size() > 0)
    {
        LOGI("Device supports the following extensions:");
        for (auto& extension : deviceExtensions)
        {
            LOGI("  \t{}", extension.extensionName);
        }
    }

    std::vector<const char*> unsupported_extensions{};
    for (auto& extension : requiredExtensions)
    {
        if (isExtensionSupported(extension.first))
        {
            enabled_extensions.emplace_back(extension.first);
        }
        else
        {
            unsupported_extensions.emplace_back(extension.first);
        }
    }

    if (enabled_extensions.size() > 0)
    {
        LOGI("Device supports the following requested extensions:");
        for (auto& extension : enabled_extensions)
        {
            LOGI("  \t{}", extension);
        }
    }

    if (unsupported_extensions.size() > 0)
    {
        auto error = false;
        for (auto& extension : unsupported_extensions)
        {
            auto extension_is_optional = requiredExtensions[extension];
            if (extension_is_optional)
            {
                LOGW("Optional device extension {} not available, some features may be disabled", extension);
            }
            else
            {
                LOGE("Required device extension {} not available, cannot run", extension);
                error = true;
            }
        }

        if (error)
        {
            std::runtime_error("Required extension missed");
        }
    }

    enabled_extensions.push_back("VK_KHR_get_memory_requirements2");
    enabled_extensions.push_back("VK_KHR_dedicated_allocation");


    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    // pointers to the queue creation info and device features structs
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pEnabledFeatures = &features;
    // enable the device extensions todo checkExtension supported
    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
    createInfo.ppEnabledExtensionNames = enabled_extensions.data();

    VK_CHECK_RESULT(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device));

    queues.resize(queueFamilyCount);
    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, queueFamilyIndex, surface, &presentSupport);
        for (uint32_t i = 0; i < queueCreateInfos[queueFamilyIndex].queueCount; i++)
        {
            const VkQueueFamilyProperties& queueFamilyProp = queueFamilyProperties[queueFamilyIndex];
            queues[queueFamilyIndex].emplace_back(
                std::move(std::make_unique<Queue>(this, queueFamilyIndex, i, presentSupport, queueFamilyProp)));
        }
    }

    commandPool = std::make_unique<CommandPool>(*this,
                                                getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0).getFamilyIndex(),
                                                CommandBuffer::ResetMode::AlwaysAllocate);

    cache = std::make_unique<ResourceCache>(*this);
}


Queue& Device::getQueueByFlag(VkQueueFlagBits requiredFlag, uint32_t queueIndex)
{
    for (const auto& queueFamily : queues)
    {
        const auto& prop = queueFamily[0]->getProp();
        auto queueFlag = prop.queueFlags;
        auto queueCount = prop.queueCount;
        if ((queueFlag & requiredFlag) && queueIndex < queueCount)
            return *queueFamily[queueIndex];
    }
    RUN_TIME_ERROR("Failed to find required queue");
    Queue* queue;
    return *queue;
    // return *(queues[0][0]);
}

const Queue& Device::getPresentQueue(uint32_t queueIndex)
{
    for (const auto& queueFamily : queues)
    {
        const auto& prop = queueFamily[0]->getProp();
        auto canPresent = queueFamily[0]->supportPresent();
        auto queueCount = prop.queueCount;
        if (canPresent && queueIndex < queueCount)
            return *queueFamily[queueIndex];
    }
    RUN_TIME_ERROR("Failed to find present queue");

    Queue* queue;
    return *queue;
    // return *(queues[0][0]);
}

ResourceCache& Device::getResourceCache()
{
    return *cache;
}

bool Device::isExtensionSupported(const std::string& extensionName)
{
    return std::find_if(deviceExtensions.begin(), deviceExtensions.end(),
                        [extensionName](auto& device_extension)
                        {
                            return std::strcmp(device_extension.extensionName, extensionName.c_str()) == 0;
                        }) != deviceExtensions.end();
}

#include "Device.h"

Device::Device(VkPhysicalDevice physicalDevice) {
//    VkDeviceCreateInfo createInfo{};
//    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//    // pointers to the queue creation info and device features structs
//    createInfo.pQueueCreateInfos = queueCreateInfos.data();
//    createInfo.queueCreateInfoCount = queueCreateInfos.size();
//    createInfo.pEnabledFeatures = &deviceFeatures;
//    // enable the device extensions
//    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
//    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
//
//
//    if (enableValidationLayers) {
//        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
//        createInfo.ppEnabledLayerNames = validationLayers.data();
//    } else {
//        createInfo.enabledLayerCount = 0;
//    }
//
//    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
//        RUN_TIME_ERROR("failed to create logical device")
}

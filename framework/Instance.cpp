//
// Created by 打工人 on 2023/3/11.
//

#include <vulkan/vulkan.h>


#include "Instance.h"
#include "Utils/DebugUtils.h"

bool enableExtension(const char *requiredExtName,
                     const std::vector<VkExtensionProperties> &availableExts,
                     std::vector<const char *> &enabledExtensions) {
    for (auto &availExtIt: availableExts) {
        if (strcmp(availExtIt.extensionName, requiredExtName) == 0) {
            auto it = std::find_if(enabledExtensions.begin(), enabledExtensions.end(),
                                   [requiredExtName](const char *enabledExtName) {
                                       return strcmp(enabledExtName, requiredExtName) == 0;
                                   });
            if (it != enabledExtensions.end()) {
                // Extension is already enabled
            } else {
                LOGI("Extension {} found, enabling it", requiredExtName);
                enabledExtensions.emplace_back(requiredExtName);
            }
            return true;
        }
    }

    LOGI("Extension {} not found", requiredExtName);
    return false;
}

bool checkLayers(const std::vector<const char *> &require, const std::vector<VkLayerProperties> &available) {
    for (const auto &requireLayer: require) {
        bool found = false;
        for (const auto &availableLayer: available) {
            if (strcmp(requireLayer, availableLayer.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    return true;
}

Instance::Instance(const std::string &application_name,
                   const std::unordered_map<const char *, bool> &required_extensions,
                   const std::vector<const char *> &required_validation_layers, bool headless, uint32_t api_version) {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No framework";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    //todo check extension support
    uint32_t extensionCount;
    std::vector<VkExtensionProperties> props;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    props.resize(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, props.data());


    for (const auto &requiredExtension: required_extensions) {
        if (!enableExtension(requiredExtension.first, props, enabledExtensions)) {
            if (requiredExtension.second) {
                LOGE("Required instance extension {} not available, cannot run", requiredExtension.first);
                RUN_TIME_ERROR("Instance extension error")
            } else {
                LOGW("Optional instance extension {} not available, some features may be disabled",
                     requiredExtension.first);
            }
        }
    }
    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = enabledExtensions.size();
    instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    if (enableValidationLayers) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        if (!checkLayers(required_validation_layers, availableLayers))
            RUN_TIME_ERROR("Required layers are missing")
        else {
            instanceInfo.enabledLayerCount = required_validation_layers.size();
            instanceInfo.ppEnabledLayerNames = required_validation_layers.data();

            VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
            fillDebugMessengerInfo(debugMessengerCreateInfo);
            instanceInfo.pNext = &debugMessengerCreateInfo;
        }
    } else {
        instanceInfo.enabledLayerCount = 0;
    }
    VK_CHECK_RESULT(vkCreateInstance(&instanceInfo, nullptr, &_instance));

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
    fillDebugMessengerInfo(debugMessengerCreateInfo);

    LOGI("Instance Created");
    // vkCreateDebugUtilsMessengerEXT(_instance,&debugMessengerCreateInfo, nullptr,&_debugMessenger);
}

void Instance::fillDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &debugUtilsMessengerInfo) {
    debugUtilsMessengerInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsMessengerInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugUtilsMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugUtilsMessengerInfo.pfnUserCallback = DebugUtils::debugCallback;
}

//std::vector<const std::string> Instance::getAvailableExtensions() {
//    uint32_t extensionCount = 0;
//    std::vector<VkExtensionProperties> props;
//    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
//    props.resize(extensionCount);
//    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, props.data());
//    std::vector<std::string> extensionNames(extensionCount);
//    std::transform(props.begin(), props.end(), extensionNames.begin(),
//                   [](const VkExtensionProperties &prop) { return prop.extensionName; });
//    return extensionNames;
//}

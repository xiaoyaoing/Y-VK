//
// Created by 打工人 on 2023/3/11.
//

#include "Instance.h"
#include "Utils/DebugUtils.h"

bool checkLayers(const std::vector<const char *> & require,const std::vector<VkLayerProperties>& available){
    for(const auto & requireLayer:require){
        bool found = false;
        for(const auto & availableLayer : available){
            if(strcmp(requireLayer,availableLayer.layerName) == 0 )
            {
                found = true;
                break;
            }
        }
        if(!found)
            return false;
    }
    return true;
}

Instance::Instance(const std::string &application_name,
                   const std::vector<const char *> &required_extensions,
                   const std::vector<const char *> &required_validation_layers, bool headless, uint32_t api_version) {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    //todo check extension support
    std::vector<const char *> availableExtensions;
    for(const auto & requiredExtension : required_extensions)
        availableExtensions.push_back(requiredExtension);
    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = 4;
    instanceInfo.ppEnabledExtensionNames = availableExtensions.data();
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    if(enableValidationLayers){
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        if(!checkLayers(required_validation_layers, availableLayers))
            RUN_TIME_ERROR("Required layers are missing")
        else {
            instanceInfo.enabledLayerCount = required_validation_layers.size();
            instanceInfo.ppEnabledLayerNames = required_validation_layers.data();

            VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
            fillDebugMessengerInfo(debugMessengerCreateInfo);
            instanceInfo.pNext = &debugMessengerCreateInfo;
        }
    }
    else {
        instanceInfo.enabledLayerCount = 0;
    }
    auto res = vkCreateInstance(&instanceInfo,nullptr,&_instance);
    ASSERT(vkCreateInstance(&instanceInfo,nullptr,&_instance)==VK_SUCCESS,"Failed to create instance");

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
    fillDebugMessengerInfo(debugMessengerCreateInfo);
    
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

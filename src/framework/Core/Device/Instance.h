#pragma once

#include "Core/Vulkan.h"
#include <unordered_map>

class Instance {
public:
    inline VkInstance getHandle() {
        return _instance;
    };

    Instance(const std::string &application_name,
             const std::unordered_map<const char *, bool> &required_extensions = {},
             const std::vector<const char *> &required_validation_layers = {},
             bool headless = false,
             uint32_t api_version = VK_API_VERSION_1_0);

protected:
//    std::vector<std::string> getAvailableExtensions();
    std::vector<const char *> enabledExtensions{};

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
#ifdef _DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

    void fillDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &debugInfo);
};

#pragma once
#include <Vulkan.h>
#include <unordered_map>
class Instance {
public:
    inline VkInstance getHandle() {
        return _instance;
    };
    Instance(const std::string &                           application_name,
             const std::vector<const char *> &required_extensions        = {},
             const std::vector<const char *> &             required_validation_layers = {},
             bool                                          headless                   = false,
             uint32_t                                      api_version                = VK_API_VERSION_1_0);
protected:
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
#ifdef  NOEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void fillDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &debugInfo);
};

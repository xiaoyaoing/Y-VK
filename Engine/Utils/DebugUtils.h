#include "Engine/Vulkan.h"
class DebugUtils {
public:
    static VKAPI_ATTR VkBool32  VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData) {
        return VK_FALSE;
    }
protected:
};

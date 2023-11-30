#include <iostream>
#include <vulkan/vulkan_core.h>

#pragma once


#define VK_CHECK_RESULT(f)                                                                                \
{                                                                                                        \
    VkResult res = (f);                                                                                    \
    if (res != VK_SUCCESS)                                                                                \
    {                                                                                                    \
        std::cout << "Fatal : VkResult is \"" << DebugUtils::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
        assert(res == VK_SUCCESS);                                                                        \
    }                                                                                                    \
}

class DebugUtils {
public:
    static std::string errorString(VkResult errorCode);

    static VKAPI_ATTR VkBool32  VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData) {
        return VK_FALSE;
    }

protected:
};



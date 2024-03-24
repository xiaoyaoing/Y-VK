#include <iostream>
#include <glm/vec4.hpp>
#include <volk.h>
#pragma once

#define VK_CHECK_RESULT(f)                                                                                                                    \
    {                                                                                                                                         \
        VkResult res = (f);                                                                                                                   \
        if (res != VK_SUCCESS) {                                                                                                              \
            std::cout << "Fatal : VkResult is \"" << DebugUtils::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
        }                                                                                                                                     \
    }

#define CHECK_RESULT(f)                                                           \
    {                                                                             \
        if (!(f)) {                                                               \
            std::cout << "Fatal \""                                               \
                      << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
        }                                                                         \
    }

class DebugUtils {
public:
    static std::string errorString(VkResult errorCode);

    // static PFN_vkCmdBeginDebugUtilsLabelEXT  vkCmdBeginDebugUtilsLabelEXT;
    // static PFN_vkCmdEndDebugUtilsLabelEXT    vkCmdEndDebugUtilsLabelEXT;
    // static PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;
    // static PFN_vkSetDebugUtilsObjectNameEXT  vkSetDebugUtilsObjectNameEXT;

    static void Setup(VkInstance instance);
    static void CmdBeginLabel(VkCommandBuffer cmd_buffer, const std::string& caption, const glm::vec4& color);
    static void CmdInsertLabel(VkCommandBuffer cmd_buffer, const std::string& caption, const glm::vec4& color);
    static void CmdEndLabel(VkCommandBuffer cmd_buffer);
    static void SetObjectName(VkDevice device, uint64_t object, VkObjectType object_type, const std::string& name);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT             messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*                                       pUserData) {
        return VK_FALSE;
    }

protected:
};
//
// Created by 打工人 on 2023/3/11.
//

#include "DebugUtils.h"
#define VK_EXT_debug_utils

std::string DebugUtils::errorString(VkResult errorCode) {
    switch (errorCode) {
#define STR(r) case VK_ ## r: return #r
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
#undef STR
        default:
            return "UNKNOWN_ERROR";
    }
}

void DebugUtils::Setup(VkInstance instance){
    
}

void DebugUtils::CmdBeginLabel(VkCommandBuffer cmd_buffer, const std::string& caption, const glm::vec4& color){
    VkDebugUtilsLabelEXT label_info{};
    label_info.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label_info.pLabelName = caption.c_str();
    memcpy(label_info.color, &color[0], sizeof(float) * 4);
    vkCmdBeginDebugUtilsLabelEXT(cmd_buffer, &label_info);
    
}void DebugUtils::CmdInsertLabel(VkCommandBuffer cmd_buffer, const std::string& caption, const glm::vec4& color){
    if (!vkCmdInsertDebugUtilsLabelEXT) {
        return;
    }
    VkDebugUtilsLabelEXT label_info{};
    label_info.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label_info.pLabelName = caption.c_str();
    memcpy(label_info.color, &color[0], sizeof(float) * 4);
    vkCmdInsertDebugUtilsLabelEXT(cmd_buffer, &label_info);
}

void DebugUtils::CmdEndLabel(VkCommandBuffer cmd_buffer){
    if (!vkCmdEndDebugUtilsLabelEXT) {
        return;
    }
    vkCmdEndDebugUtilsLabelEXT(cmd_buffer);
}
void DebugUtils::SetObjectName(VkDevice device, uint64_t object, VkObjectType object_type, const std::string& name){
    if (!vkSetDebugUtilsObjectNameEXT) {
        return;
    }
    VkDebugUtilsObjectNameInfoEXT name_info{};
    name_info.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType   = object_type;
    name_info.objectHandle = object;
    name_info.pObjectName  = name.c_str();
    vkSetDebugUtilsObjectNameEXT(device, &name_info);
}

//
// Created by pc on 2023/8/12.
//
#include "VkUtils.h"

bool isDepthOnlyFormat(VkFormat format) {
    return format == VK_FORMAT_D16_UNORM ||
           format == VK_FORMAT_D32_SFLOAT;
}

bool isDepthOrStencilFormat(VkFormat format) {
    return format == VK_FORMAT_D16_UNORM_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT ||
           format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           isDepthOnlyFormat(format);
}
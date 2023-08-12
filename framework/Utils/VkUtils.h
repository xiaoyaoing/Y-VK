#include <vulkan/vulkan_core.h>

#pragma once

bool isDepthOnlyFormat(VkFormat format);

bool isDepthOrStencilFormat(VkFormat format);

struct ImageMemoryBarrier {
    VkImageLayout oldLayout, newLayout;
    VkAccessFlags srcAccessMask, dstAccessMask;
    VkPipelineStageFlags srcStageMask, dstStageMask;
    uint32_t oldQueueFamily{VK_QUEUE_FAMILY_IGNORED}, newQueueFamily{VK_QUEUE_FAMILY_IGNORED};
};
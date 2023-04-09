#include <Engine/Vulkan.h>


bool isDepthOnlyFormat(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM ||
           format == VK_FORMAT_D32_SFLOAT;
}

bool isDepthOrStencilFormat(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT ||
           format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           isDepthOnlyFormat(format);
}

struct ImageMemoryBarrier{
    VkImageLayout oldLayout,newLayout;
    VkAccessFlags srcAccessMask,dstAccessMask;
    VkPipelineStageFlags srcStageMask,dstStageMask;
    uint32_t oldQueueFamily{VK_QUEUE_FAMILY_IGNORED},newQueueFamily{VK_QUEUE_FAMILY_IGNORED};
};
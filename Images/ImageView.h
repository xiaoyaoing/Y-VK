#pragma once
#include "Vulkan.h"
class Image;
class Device;
class ImageView {
public:
    inline VkImageView getHandle() {
        return _view;
    }
    ImageView( ptr<Device> & device, const ptr<Image> & targetImage,
              VkImageAspectFlags aspect, uint32_t mipLevels);
    static VkImageViewCreateInfo getDefaultImageViewInfo()
    {
        VkImageViewCreateInfo depthViewInfo = {};
        depthViewInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthViewInfo.pNext			= nullptr;
        depthViewInfo.components.r	= VK_COMPONENT_SWIZZLE_IDENTITY;
        depthViewInfo.components.g	= VK_COMPONENT_SWIZZLE_IDENTITY;
        depthViewInfo.components.b	= VK_COMPONENT_SWIZZLE_IDENTITY;
        depthViewInfo.components.a	= VK_COMPONENT_SWIZZLE_IDENTITY;
        depthViewInfo.subresourceRange.baseMipLevel		= 0;
        depthViewInfo.subresourceRange.levelCount		= 1;
        depthViewInfo.subresourceRange.baseArrayLayer	= 0;
        depthViewInfo.subresourceRange.layerCount		= 1;
        return depthViewInfo;
    }
protected:
    VkImageView _view;
    ptr<Device> _device;
};

#pragma once

#include "Vulkan.h"

class Image;

class Device;

class ImageView {
public:
    inline VkImageView getHandle() const {
        return _view;
    }

    inline Image &getImage() const {
        return image;
    }

//    ImageView( ptr<Device> & device, const ptr<Image> & targetImage,
//              VkImageAspectFlags aspect, uint32_t mipLevels);
    ImageView(Image &image, VkImageViewType view_type, VkFormat format = VK_FORMAT_UNDEFINED,
              uint32_t mip_level = 0, uint32_t base_array_layer = 0,
              uint32_t n_mip_levels = 0, uint32_t n_array_layers = 0);

    const VkImageSubresourceRange &getSubResourceRange() const;

    static VkImageViewCreateInfo getDefaultImageViewInfo() {
        VkImageViewCreateInfo depthViewInfo = {};
        depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthViewInfo.pNext = nullptr;
        depthViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthViewInfo.subresourceRange.baseMipLevel = 0;
        depthViewInfo.subresourceRange.levelCount = 1;
        depthViewInfo.subresourceRange.baseArrayLayer = 0;
        depthViewInfo.subresourceRange.layerCount = 1;
        return depthViewInfo;
    }

protected:
    VkImageView _view;
    Image &image;
    Device &_device;
    VkFormat _format;
    VkImageSubresourceRange _subResourceRange;
};

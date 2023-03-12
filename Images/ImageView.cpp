//
// Created by 打工人 on 2023/3/6.
//
#include "ImageView.h"
#include <Device.h>
#include <Images/Image.h>
ImageView::ImageView(ptr<Device> &device, const ptr<Image> &targetImage, VkImageAspectFlags aspect,
                     uint32_t mipLevels) {
    _device = device;

    VkImageViewCreateInfo imageViewInfo				= {};
    imageViewInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.pNext								= nullptr;
    imageViewInfo.viewType							= static_cast<VkImageViewType>(targetImage->getImageType());
    imageViewInfo.format							= targetImage->getFormat();
    imageViewInfo.image								= targetImage->getHandle();
    imageViewInfo.components.r						= VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g						= VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b						= VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a						= VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.subresourceRange.aspectMask		= aspect;
    imageViewInfo.subresourceRange.baseMipLevel		= 0;
    imageViewInfo.subresourceRange.levelCount		= mipLevels;
    imageViewInfo.subresourceRange.baseArrayLayer	= 0;
    imageViewInfo.subresourceRange.layerCount		= 1;

    ASSERT(vkCreateImageView(_device->getHandle(), &imageViewInfo, nullptr, &_view) == VK_SUCCESS,"create imageView");
}

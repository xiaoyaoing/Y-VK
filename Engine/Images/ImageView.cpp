//
// Created by 打工人 on 2023/3/6.
//
#include "ImageView.h"
#include "Engine/Device.h"
#include "Image.h"
#include <Engine/Utils/VkUtils.h>
ImageView::ImageView(ptr<Device> &device, const ptr<Image> &targetImage, VkImageAspectFlags aspect,
                     uint32_t mipLevels) {
    _device = device;

    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.pNext = nullptr;
    imageViewInfo.viewType = static_cast<VkImageViewType>(targetImage->getImageType());
    imageViewInfo.format = targetImage->getFormat();
    imageViewInfo.image = targetImage->getHandle();
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.subresourceRange.aspectMask = aspect;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = mipLevels;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    ASSERT(vkCreateImageView(_device->getHandle(), &imageViewInfo, nullptr, &_view) == VK_SUCCESS, "create imageView");
}

ImageView::ImageView(Image &image, VkImageViewType view_type, VkFormat format, uint32_t base_mip_level,
                     uint32_t base_array_layer, uint32_t n_mip_levels, uint32_t n_array_layers) : _image(&image),
                                                                                                  _format{format} {


    if (format == VK_FORMAT_UNDEFINED) {
       _format = format = image.getFormat();
    }
    //todo config this 
//    _subResourceRange.
//            baseMipLevel = mip_level;
//    _subResourceRange.
//            baseArrayLayer = array_layer;
//    _subResourceRange.
//            levelCount = n_mip_levels == 0 ? image->get_subresource().mipLevel : n_mip_levels;
//    _subResourceRange.
//            layerCount = n_array_layers == 0 ? image->get_subresource().arrayLayer : n_array_layers;
//
//    if (
//            is_depth_stencil_format(format)
//            ) {
//        _subResourceRange.
//                aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//    } else {
//        _subResourceRange.
//                aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    }

    VkImageViewCreateInfo imageViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    imageViewInfo.image = image.getHandle();
    imageViewInfo.viewType = view_type;
    imageViewInfo.
            format = format;
    imageViewInfo.subresourceRange.aspectMask = isDepthOrStencilFormat(format)?VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = n_mip_levels;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    VK_VERIFY_RESULT(vkCreateImageView(_device->getHandle(), &imageViewInfo, nullptr, &_view));
    image.addView(this);
}

const VkImageSubresourceRange &ImageView::getSubResourceRange() const {
    return _subResourceRange;
}
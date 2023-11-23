//
// Created by 打工人 on 2023/3/6.
//
#include "ImageView.h"
#include "Device.h"
#include "Image.h"
#include <Utils/VkUtils.h>
//ImageView::ImageView(ptr<Device> &device, const ptr<Image> &targetImage, VkImageAspectFlags aspect,
//                     uint32_t mipLevels) {
//    _device = device;
//
//    VkImageViewCreateInfo imageViewInfo = {};
//    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//    imageViewInfo.pNext = nullptr;
//    imageViewInfo.viewType = static_cast<VkImageViewType>(targetImage->getImageType());
//    imageViewInfo.format = targetImage->getFormat();
//    imageViewInfo.image = targetImage->getHandle();
//    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//    imageViewInfo.subresourceRange.aspectMask = aspect;
//    imageViewInfo.subresourceRange.baseMipLevel = 0;
//    imageViewInfo.subresourceRange.levelCount = mipLevels;
//    imageViewInfo.subresourceRange.baseArrayLayer = 0;
//    imageViewInfo.subresourceRange.layerCount = 1;
//
//    ASSERT(vkCreateImageView(_device.getHandle(), &imageViewInfo, nullptr, &_view) == VK_SUCCESS, "create imageView");
//}

ImageView::ImageView(Image& image, VkImageViewType view_type, VkFormat format, uint32_t mip_level,
                     uint32_t base_array_layer, uint32_t n_mip_levels, uint32_t n_array_layers) : image(image),
    _device(image.getDevice()),
    _format{format}
{
    if (format == VK_FORMAT_UNDEFINED)
    {
        _format = format = image.getFormat();
    }
    //


    //    //todo config this
    subResourceRange.
        baseMipLevel = mip_level;
    subResourceRange.
        baseArrayLayer = base_array_layer;
    subResourceRange.
        levelCount = n_mip_levels == 0 ? image.getSubresource().mipLevel : n_mip_levels;
    subResourceRange.
        layerCount = n_array_layers == 0 ? image.getSubresource().arrayLayer : n_array_layers;

    if (isDepthOnlyFormat(format))
    {
        subResourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else
    {
        subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageViewCreateInfo imageViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

    imageViewInfo.image = image.getHandle();
    imageViewInfo.viewType = view_type;
    imageViewInfo.
        format = format;
    imageViewInfo.subresourceRange = subResourceRange;

    VK_CHECK_RESULT(vkCreateImageView(_device.getHandle(), &imageViewInfo, nullptr, &_view));
    image.addView(this);
}

VkFormat ImageView::getFormat() const
{
    return _format;
}

const VkImageSubresourceRange& ImageView::getSubResourceRange() const
{
    return subResourceRange;
}

ImageView::~ImageView()
{
    vkDestroyImageView(_device.getHandle(), _view, nullptr);
}

ImageView::ImageView(ImageView&& other) : image(other.image), _device(other._device), _format(other._format),
                                          subResourceRange(other.subResourceRange)
{
    //    auto &views = image->get_views();
    //    views.erase(&other);
    //    views.emplace(this);

    other._view = VK_NULL_HANDLE;
}

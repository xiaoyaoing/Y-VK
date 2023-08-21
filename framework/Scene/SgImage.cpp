//
// Created by pc on 2023/8/18.
//

#include "SgImage.h"
#include "Images/Image.h"
#include "Images/ImageView.h"
#include "Device.h"
#include "Scene/Images/StbImage.h"
#include <FIleUtils.h>

namespace sg {
    std::unique_ptr<SgImage> SgImage::load(const std::string &path) {
        auto ext = FileUtils::getFileExt(path);
        if (ext == "jpg" || ext == "png") {
            return std::make_unique<StbImage>(path);
        }
        return std::unique_ptr<SgImage>();
    }

    void SgImage::createVkImage(Device &device) {
        vkImage = std::make_unique<Image>(device, extent3D, format,
                                          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY);
        vkImageView = std::make_unique<ImageView>(*vkImage, VK_IMAGE_VIEW_TYPE_2D);
    }

    uint64_t SgImage::getBufferSize() const {
        return data.size();
    }

    std::vector<uint8_t> &SgImage::getData() {
        return data;
    }

//    void SgImage::createImage(Device &device) {
//
//    }

//    void SgImage::createVkImage(Device &device)
//    {
//        vkImage = std::make_unique<Image>(device, format, extent3D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
//        vkImageView = std::make_unique<ImageView>(*vkImage, VK_IMAGE_VIEW_TYPE_2D);
//    }

    VkExtent3D SgImage::getExtent() const {
        return extent3D;
    }

    Image &SgImage::getVkImage() {
        return *vkImage;
    }

    ImageView &SgImage::getVkImageView() {
        return *vkImageView;
    }
}

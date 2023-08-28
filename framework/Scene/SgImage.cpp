//
// Created by pc on 2023/8/18.
//

#include "SgImage.h"
#include "Images/Image.h"
#include "Images/ImageView.h"
#include "Device.h"
#include "Scene/Images/StbImage.h"
#include "Scene/Images/TtfImage.h"
#include "Scene/Images/KtxImage.h"
#include <FIleUtils.h>

namespace sg {
    std::unique_ptr<SgImage> SgImage::load(const std::string &path) {
        auto ext = FileUtils::getFileExt(path);
        if (ext == "jpg" || ext == "png") {
            return std::make_unique<StbImage>(path);
        } else if (ext == "ttf") {
            return std::make_unique<TtfImage>(path);
        } else if (ext == "ktx") {
            return std::make_unique<KtxImage>(path);
        }
        return std::unique_ptr<SgImage>();
    }

    void SgImage::createVkImage(Device &device, VkImageViewType image_view_type, VkImageCreateFlags flags) {
        vkImage = std::make_unique<Image>(device, extent3D, format,
                                          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY, VK_SAMPLE_COUNT_1_BIT, toUint32(mipMaps.size()),
                                          layers,
                                          VK_IMAGE_TILING_OPTIMAL, flags);
        vkImageView = std::make_unique<ImageView>(*vkImage, image_view_type);
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

    const std::vector<Mipmap> &SgImage::getMipMaps() const {
        return mipMaps;
    }

    const std::vector<std::vector<VkDeviceSize>> &SgImage::getOffsets() const {
        return offsets;
    }

    uint32_t SgImage::getLayers() const {
        return layers;
    }

    void SgImage::setLayers(uint32_t layers) {
        SgImage::layers = layers;
    }
}

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

    void SgImage::generateMipMap() {
        assert(mipMaps.size() == 1 && "Mipmaps already generated");

        auto extent = getExtent();
        auto next_width = std::max<uint32_t>(1u, extent.width / 2);
        auto next_height = std::max<uint32_t>(1u, extent.height / 2);
        auto channels = 4;
        auto next_size = next_width * next_height * channels;

        while (true) {
            // Make space for next mipmap
            auto old_size = toUint32(data.size());
            data.resize(old_size + next_size);

            auto &prev_mipmap = mipMaps.back();
            // Update mipmaps
            Mipmap next_mipmap{};
            next_mipmap.level = prev_mipmap.level + 1;
            next_mipmap.offset = old_size;
            next_mipmap.extent = {next_width, next_height, 1u};

            //todo

            // std::copy(data.begin()+ next_mipmap.offset,data.begin()+prev_mipmap.offset,data.begin()+pr

            // Fill next mipmap memory
//            stbir_resize_uint8(data.data() + prev_mipmap.offset, prev_mipmap.extent.width, prev_mipmap.extent.height, 0,
//                               data.data() + next_mipmap.offset, next_mipmap.extent.width, next_mipmap.extent.height, 0,
//                               channels);

            mipMaps.emplace_back(std::move(next_mipmap));

            // Next mipmap values
            next_width = std::max<uint32_t>(1u, next_width / 2);
            next_height = std::max<uint32_t>(1u, next_height / 2);
            next_size = next_width * next_height * channels;

            if (next_width == 1 && next_height == 1) {
                break;
            }
        }
    }
}

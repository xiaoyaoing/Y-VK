//
// Created by pc on 2023/8/18.
//
#pragma once

#include "Vulkan.h"

#include "Images/ImageView.h"
#include "Images/Image.h"

namespace sg {
    struct Mipmap {
        /// Mipmap level
        uint32_t level = 0;

        /// Byte offset used for uploading
        uint32_t offset = 0;

        /// Width depth and height of the mipmap
        VkExtent3D extent = {0, 0, 0};
    };

    class SgImage {
    public:
        static std::unique_ptr<SgImage> load(const std::string &path);

        void createVkImage(Device &device, VkImageViewType image_view_type = VK_IMAGE_VIEW_TYPE_2D,
                           VkImageCreateFlags flags = 0);

        std::vector<uint8_t> &getData();

        uint64_t getBufferSize() const;

        VkExtent3D getExtent() const;

        Image &getVkImage();

        ImageView &getVkImageView();

        const std::vector<std::vector<VkDeviceSize>> &getOffsets() const;

        const std::vector<Mipmap> &getMipMaps() const;

        uint32_t getLayers() const;

        void setLayers(uint32_t layers);


    protected:
        std::unique_ptr<Image> vkImage;

        std::unique_ptr<ImageView> vkImageView;

        std::vector<uint8_t> data;

        VkFormat format;

        VkExtent3D extent3D;


        std::vector<Mipmap> mipMaps{{}};

        std::vector<std::vector<VkDeviceSize>> offsets;

    protected:
        uint32_t layers{1};


    };
}
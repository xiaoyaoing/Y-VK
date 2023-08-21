//
// Created by pc on 2023/8/18.
//
#pragma once

#include "Vulkan.h"

#include "Images/ImageView.h"
#include "Images/Image.h"

namespace sg
{
    class SgImage
    {
    public:
        static std::unique_ptr<SgImage> load(const std::string &path);

        void createVkImage(Device &device);

        std::vector<uint8_t> &getData();

        uint64_t getBufferSize() const;

        VkExtent3D getExtent() const;

        Image &getVkImage();

        ImageView &getVkImageView();

    protected:
        std::unique_ptr<Image> vkImage;

        std::unique_ptr<ImageView> vkImageView;

        std::vector<uint8_t> data;

        VkFormat format;

        VkExtent3D extent3D;
    };

}
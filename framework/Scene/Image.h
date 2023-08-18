//
// Created by pc on 2023/8/18.
//
#pragma once

#include "Vulkan.h"

namespace sg {
    class Image {
    public:
        static std::unique_ptr<Image> load(const std::string &path);

        void createVkImage();

    protected:
        VkImage vkImage;
        VkImageView vkImageView;
    };

    std::vector<uint8_t> data;

}
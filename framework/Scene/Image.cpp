//
// Created by pc on 2023/8/18.
//

#include "Image.h"

namespace sg {
    std::unique_ptr<Image> sg::Image::load(const std::string &path) {
        return std::unique_ptr<Image>();
    }

    void sg::Image::createVkImage() {
        VkImageCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};

    }

}

//
// Created by pc on 2023/8/18.
//

#pragma once

#include "Vulkan.h"
#include "Scene/Image.h"

struct Texture {
    std::unique_ptr<sg::Image> image;
    VkSampler sampler;
};
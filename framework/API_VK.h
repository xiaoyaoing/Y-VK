//
// Created by pc on 2023/8/18.
//

#pragma once

#include "Vulkan.h"
#include "Scene/SgImage.h"
#include "Images/Sampler.h"

struct Texture {
    std::unique_ptr<sg::SgImage> image;
    std::unique_ptr<Sampler> sampler;
};
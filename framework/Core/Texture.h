//
// Created by pc on 2023/8/18.
//

#pragma once

#include "Core/Vulkan.h"
#include "Scene/SgImage.h"
#include "Images/Sampler.h"

struct Texture {
    std::unique_ptr<SgImage> image;
    std::unique_ptr<Sampler> sampler;
    std::string name;

    const Sampler &getSampler() const;

    const SgImage &getImage() const;

    static Texture loadTexture(Device &device, const std::string &path);

    static Texture loadTextureArray(Device &device, const std::string &path);
};

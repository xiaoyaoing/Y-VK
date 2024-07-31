//
// Created by pc on 2023/8/18.
//

#pragma once

#include "Core/Vulkan.h"
#include "Scene/SgImage.h"
#include "Images/Sampler.h"

struct Texture {
    std::unique_ptr<SgImage> image{nullptr};
    const Sampler*           sampler{nullptr};
    std::string              name;

    const Sampler& getSampler() const;

    const SgImage& getImage() const;

    static std::unique_ptr<Texture> loadTextureFromFile(Device& device, const std::string& path);
    static std::unique_ptr<Texture> loadTextureFromFileWitoutInit(Device& device, const std::string& path);
    static std::unique_ptr<Texture> loadTextureFromMemoryWithoutInit(Device& device, std::vector<uint8_t>& data, VkExtent3D extent, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);

    static std::unique_ptr<Texture> loadTextureFromMemory(Device& device, std::vector<uint8_t>& data, VkExtent3D extent, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    static std::unique_ptr<Texture> loadTextureArrayFromFile(Device& device, const std::string& path);
    static void                     initTexturesInOneSubmit(std::vector<std::unique_ptr<Texture>>& textures);
    Texture&                        operator=(Texture&&);
    Texture(Texture& texture) = delete;
    Texture()                 = default;
};

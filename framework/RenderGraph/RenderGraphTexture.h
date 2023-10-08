#pragma once

#include <memory>
#include <vulkan/vulkan_core.h>

#include "vk_mem_alloc.h"
#include "Scene/SgImage.h"


class Device;

class ImageView;

class Image;


class Texture;


/*
 * Texture used in RenderGraph.
 * Real hardware texture only created when required.
 */
class RenderGraphTexture {
    sg::SgImage *hwTexture{nullptr};
    //
    // Image* image{nullptr};
    // ImageView* imageView{nullptr};

public:
    struct Descriptor {
        //Device &device;
        VkExtent2D extent{};
        VkFormat format;
        VkImageUsageFlags usageFlags;
        VmaMemoryUsage memoryUsage;
    };

    RenderGraphTexture() = default;

    void setHwTexture(sg::SgImage *hwTexture);

    sg::SgImage *getHwTexture() const;

    // const HwTexture& getHandle() const;

    void create(const char *name,
                const Descriptor &descriptor);
};

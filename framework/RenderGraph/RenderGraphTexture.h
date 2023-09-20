#pragma once
#include <memory>
#include <vulkan/vulkan_core.h>


class ImageView;
class Image;

struct HwTexture
{
    std::unique_ptr<ImageView> view;
    std::unique_ptr<Image> image;
};

class Texture;


/*
 * Texture used in RenderGraph.
 * Real hardware texture only created when required.
 */
class RenderGraphTexture
{
    std::unique_ptr<Texture> hwTexture;

public:
    struct Descriptor
    {
        VkExtent2D extent;
        VkFormat format;
        VkImageUsageFlags usageFlags;
    };

    HwTexture * getHandle() const;

    void create(const char* name,
                const Descriptor& descriptor);
};

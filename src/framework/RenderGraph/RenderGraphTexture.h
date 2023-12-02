#pragma once

#include <memory>


#include "RenderGraphNode.h"
#include "Scene/SgImage.h"


class Device;

class ImageView;

class Image;

class Texture;

class PassNode;

/*
 * Texture used in RenderGraph.
 * Real hardware texture only created when required.
 */
class RenderGraphTexture : public RenderGraphNode
{
    SgImage* mHwTexture{nullptr};
    //
    // Image* image{nullptr};
    // ImageView* imageView{nullptr};

public:
    using Usage = TextureUsage;

    struct Descriptor
    {
        //Device &device;
        VkExtent2D extent{};
        Usage useage;
        // VkFormat format = VK_FORMAT_UNDEFINED;
        // VkImageUsageFlags usageFlags;
        // VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    };

    static constexpr Usage DEFAULT_R_USAGE = Usage::COLOR_ATTACHMENT;
    static constexpr Usage DEFAULT_W_USAGE = Usage::COLOR_ATTACHMENT;

    RenderGraphTexture() = default;

    RenderGraphTexture(const char* name, SgImage* hwTexture);
    RenderGraphTexture(const char* name, const Descriptor& descriptor);


    void setHwTexture(SgImage* hwTexture);

    SgImage* getHwTexture() const;

    // const HwTexture& getHandle() const;

    void create(const char* name,
                const Descriptor& descriptor);

    void devirtualize();

    void destroy();

    void resolveTextureUsage(CommandBuffer& commandBuffer);

public:
    bool imported{false};

    const char* mName;
    const Descriptor mDescriptor;

    PassNode *first{nullptr}, *last{nullptr};

    Usage usage{};
};

#pragma once

#include <memory>


#include "ResourceNode.h"
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
class RenderGraphTexture : public ResourceNode
{
    SgImage* mHwTexture{nullptr};
public:
    using Usage = TextureUsage;

    struct Descriptor
    {
        VkExtent2D extent{};
        Usage useage;
    };

    static constexpr Usage DEFAULT_R_USAGE = Usage::READ_ONLY;
    static constexpr Usage DEFAULT_DEPTH_R_USAGE = Usage::DEPTH_READ_ONLY;
    
    static constexpr Usage DEFAULT_W_USAGE = Usage::COLOR_ATTACHMENT;
    static constexpr Usage DEFAULT_DEPTH_W_USAGE = Usage::DEPTH_ATTACHMENT;

    RenderGraphTexture() = default;

    RenderGraphTexture(const char* name, SgImage* hwTexture);
    RenderGraphTexture(const char* name, const Descriptor& descriptor);
    
    bool isDepthStencilTexture() const;
    
    void setHwTexture(SgImage* hwTexture);

    SgImage* getHwTexture() const;

    // const HwTexture& getHandle() const;

    void create(const char* name,
                const Descriptor& descriptor);

    void devirtualize() override;

    void destroy() override;

    RENDER_GRAPH_RESOURCE_TYPE getType() const override;

    void resloveUsage(CommandBuffer& commandBuffer) override;

    void resolveTextureUsage(CommandBuffer& commandBuffer);

public:
    bool imported{false};

    const char* mName;
    const Descriptor mDescriptor;


    // Usage usage{};
};

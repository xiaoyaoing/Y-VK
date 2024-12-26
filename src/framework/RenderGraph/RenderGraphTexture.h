#pragma once

#include "Enum.h"

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
class RenderGraphTexture : public ResourceNode {
    SgImage* mHwTexture{nullptr};

public:
    using Usage = TextureUsage;

    struct Descriptor {
        VkExtent2D extent{};
        Usage      useage;
        VkFormat  format = VK_FORMAT_UNDEFINED;
    };

    static constexpr Usage DEFAULT_R_USAGE       = Usage::READ_ONLY;
    static constexpr Usage DEFAULT_DEPTH_R_USAGE = Usage::DEPTH_READ_ONLY;

    static constexpr Usage DEFAULT_W_USAGE       = Usage::COLOR_ATTACHMENT;
    static constexpr Usage DEFAULT_DEPTH_W_USAGE = Usage::DEPTH_ATTACHMENT;
    
    ~RenderGraphTexture() override;

    RenderGraphTexture(const std::string& name, SgImage* hwTexture);
    RenderGraphTexture(const std::string& name, const Descriptor& descriptor);
    
    bool isDepthStencilTexture() const;

    void setHwTexture(SgImage* hwTexture);

    SgImage* getHwTexture() const;

    // const HwTexture& getHandle() const;
    

    void devirtualize() override;

    void destroy() override;

    RenderResourceType getType() const override;

    void             resloveUsage(ResourceBarrierInfo& barrierInfo, uint16_t lastUsage, uint16_t nextUsage, RenderPassType lastPassType, RenderPassType nextPassType) override;
    uint16_t         getDefaultUsage(uint16_t nextUsage) override;
    
    bool             imported{false};
    const Descriptor mDescriptor;

    // Usage usage{};
};
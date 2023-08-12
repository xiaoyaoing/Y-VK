//
// Created by pc on 2023/8/4.
//
#include <Vulkan.h>
#include "CommandBuffer.h"

#pragma once

class RenderTarget;


class Scene;

class Subpass {
public:
    void updateRenderTargetAttachments(RenderTarget &renderTarget);

    virtual void draw(CommandBuffer &commandBuffer) = 0;

    const std::string &getDebugName() const;

    void setDebugName(const std::string &debugName);

    VkResolveModeFlagBits getDepthStencilResolveMode() const;

    void setDepthStencilResolveMode(VkResolveModeFlagBits depthStencilResolveMode);

    const std::vector<uint32_t> &getInputAttachments() const;

    void setInputAttachments(const std::vector<uint32_t> &inputAttachments);

    const std::vector<uint32_t> &getOutputAttachments() const;

    void setOutputAttachments(const std::vector<uint32_t> &outputAttachments);

    const std::vector<uint32_t> &getColorResolveAttachments() const;

    void setColorResolveAttachments(const std::vector<uint32_t> &colorResolveAttachments);

    uint32_t getDepthStencilResolveAttachment() const;

    void setDepthStencilResolveAttachment(uint32_t depthStencilResolveAttachment);

    bool getDisableDepthStencilAttachment() const;

    void setDisableDepthStencilAttachment(bool disableDepthStencilAttachment);

private:
    std::string debugName{};
    VkResolveModeFlagBits depthStencilResolveMode{VK_RESOLVE_MODE_NONE};

/// Default to no input attachments
    std::vector<uint32_t> inputAttachments = {};

/// Default to swapchain output attachment
    std::vector<uint32_t> outputAttachments = {0};

/// Default to no color resolve attachments
    std::vector<uint32_t> colorResolveAttachments = {};

/// Default to no depth stencil resolve attachment
    uint32_t depthStencilResolveAttachment{VK_ATTACHMENT_UNUSED};

    bool disableDepthStencilAttachment{false};


};


class GeomSubpass : public Subpass {
public:
    virtual void draw(CommandBuffer &commandBuffer) override;

    GeomSubpass(Scene &scene);

private:
    Scene &scene;
};



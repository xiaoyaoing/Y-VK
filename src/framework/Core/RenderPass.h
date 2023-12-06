#pragma once

#include "Core/Vulkan.h"

class Device;

struct Attachment;

struct SubpassInfo {
    std::vector<uint32_t> inputAttachments{};

    std::vector<uint32_t> outputAttachments{};

    std::vector<uint32_t> colorResolveAttachments{};

    bool disableDepthStencilAttachment{false};

    uint32_t depthStencilResolveAttachment{};

    VkResolveModeFlagBits depthStencilResolveMode{};

    std::string debugName;
};


class RenderPass {
public:
    RenderPass(RenderPass &&other);

    VkRenderPass getHandle() const {
        return _pass;
    }
    

    RenderPass(Device &device,
               const std::vector<Attachment> &attachments,
               const std::vector<SubpassInfo> &subpasses);

    const uint32_t getColorOutputCount(uint32_t subpass_index) const;

    const std::vector<VulkanLayout> & getAttachmentFinalLayouts() const;

    ~RenderPass();

    std::vector<std::vector<std::pair<uint32_t,VulkanLayout>>> subpassInputLayouts;


protected:
    VkRenderPass _pass;
    Device &device;
    std::vector<uint32_t> colorOutputCount;
    std::vector<VulkanLayout> attachmentFinalLayouts;
};
